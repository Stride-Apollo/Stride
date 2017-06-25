/**
 * @file
 * Implementation of tests for the Population Generator.
 */

#include <gtest/gtest.h>

#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "sim/LocalSimulatorAdapter.h"
#include "util/async.h"
#include "util/ConfigInfo.h"
#include "util/InstallDirs.h"
#include "util/etc.h"
#include "core/Cluster.h"

#include <boost/property_tree/xml_parser.hpp>
#include <memory>
#include <cassert>
#include <string>
#include <vector>
#include <future>
#include <utility>
#include <algorithm>

using namespace std;
using namespace stride;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;

namespace Tests {

class UnitTests__MR_SimulatorTest: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {

	}

protected:
	std::shared_ptr<Simulator>					m_sim1;
	std::shared_ptr<Simulator>					m_sim2;

	std::unique_ptr<LocalSimulatorAdapter>		m_l1;
	std::unique_ptr<LocalSimulatorAdapter>		m_l2;
	vector<unsigned int>						m_ids;
	const Simulator::PersonType*				m_first_person;

protected:
	/// Destructor has to be virtual.
	virtual ~UnitTests__MR_SimulatorTest() {}

	/// Set up for the test fixture
	virtual void SetUp() {
		boost::property_tree::ptree config_tree;
		config_tree.put("run.<xmlattr>.name", "testHdf5");

		config_tree.put("run.r0", 11.0);
		config_tree.put("run.start_date", "2017-01-01");
		config_tree.put("run.num_days", 50U);
		config_tree.put("run.holidays", "holidays_none.json");
		config_tree.put("run.age_contact_matrix_file", "contact_matrix_average.xml");
		config_tree.put("run.track_index_case", 0);
		config_tree.put("run.num_threads", 4);
		config_tree.put("run.information_policy", "Global");

		config_tree.put("run.outputs.log.<xmlattr>.level", "None");
		config_tree.put("run.outputs.participants_survey.<xmlattr>.num", 10);
		config_tree.put("run.outputs.checkpointing.<xmlattr>.frequency", 1);

		config_tree.put("run.disease.seeding_rate", 0.002);
		config_tree.put("run.disease.immunity_rate", 0.8);
		config_tree.put("run.disease.config", "disease_measles.xml");

		config_tree.put("run.regions.region.<xmlattr>.name", "Belgium");
		config_tree.put("run.regions.region.rng_seed", 1U);
		config_tree.put("run.regions.region.population", "bigpop.xml");

		// Create simulators
		m_sim1 = SimulatorBuilder::build(config_tree);
		m_sim2 = SimulatorBuilder::build(config_tree);
		m_l1 = make_unique<LocalSimulatorAdapter>(m_sim1);
		m_l2 = make_unique<LocalSimulatorAdapter>(m_sim2);

		m_sim1->setName("1");
		m_sim2->setName("2");

		map<string, AsyncSimulator*> sender_map1;
		sender_map1["2"] = m_l2.get();

		map<string, AsyncSimulator*> sender_map2;
		sender_map2["1"] = m_l1.get();

		m_sim1->setCommunicationMap(sender_map1);
		m_sim2->setCommunicationMap(sender_map2);

		// Migrate 10 people for 10 days
		m_sim1->sendNewTravellers(10, 10, "2", "Antwerp", "ANR");

		// Keep a vector of id's of people who are on vacation
		vector<unsigned int> m_ids;

		for (auto& traveller: *(m_sim2->getPlanner().getDay(10))) {
			m_ids.push_back(traveller->getHomePerson().getId());
		}

		EXPECT_EQ(m_ids.size(), 10U);

		m_first_person = &(m_sim2->getPopulation()->m_original.at(0));
	}

	/// Tearing down the test fixture
	virtual void TearDown() {}
};

TEST_F(UnitTests__MR_SimulatorTest, returnHome) {
	for (unsigned int i = 0; i < 11; i++) {
		// Test whether they are present in the population of the hosting region
		EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getDay(10 - i)->size(), 10U);
		for (unsigned int j = 0; j < m_sim2->getPopulation()->m_visitors.getDay(10 - i)->size(); ++j) {
			// They shouldn't be on vacation
			const Simulator::PersonType& person = *(m_sim2->getPopulation()->m_visitors.getDay(10 - i)->at(j));
			EXPECT_FALSE(person.isOnVacation());
		}

		m_sim1->timeStep();
		m_sim2->timeStep();
		
		m_sim1->returnForeignTravellers();
		m_sim2->returnForeignTravellers();

		for (unsigned int j = 0; j < m_sim1->getPlanner().getAgenda().size(); ++j) {
			auto it = m_sim2->getPlanner().getAgenda().begin();
			auto block = (*(next(it, j))).get();

			// Test whether they are present in the planner hosting region
			if (j == m_sim2->getPlanner().getAgenda().size() - 1) {
				EXPECT_EQ(block->size(), 10U);
			} else {
				EXPECT_EQ(block->size(), 0U);
			}
		}
	}

	ASSERT_EQ(&(m_sim2->getPopulation()->m_original.at(0)), m_first_person);

	// The agendas are empty now (or at least, it should be)
	EXPECT_EQ(m_sim2->getPlanner().getAgenda().size(), 0U);
	EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getAgenda().size(), 0U);

	// Test whether the population in both manipulated sims is not on vacation

	for (uint i = 0; i < m_sim1->getPopulation()->m_original.size(); ++i) {
		auto& person = m_sim1->getPopulation()->m_original.at(i);
		EXPECT_FALSE(person.isOnVacation());
	}

	for (uint i = 0; i < m_sim2->getPopulation()->m_original.size(); ++i) {
		auto& person = m_sim2->getPopulation()->m_original.at(i);
		EXPECT_FALSE(person.isOnVacation());
	}
}

TEST_F(UnitTests__MR_SimulatorTest, peopleMoved) {
	// Test if the people arrived in the destination simulator
	for (unsigned int i = 0; i < m_sim2->getPopulation()->m_visitors.getAgenda().size(); ++i) {
		auto it = m_sim2->getPopulation()->m_visitors.getAgenda().begin();
		auto block = (*(next(it, i))).get();
		if (i == 10) {
			ASSERT_EQ(block->size(), 10U);
			// These people can't be on vacation
			for (auto& person: *block) {
				EXPECT_FALSE(person->isOnVacation());
			}
		} else {
			EXPECT_EQ(block->size(), 0U);
		}
	}

	// Test if the people are absent in the home simulator
	for (auto id: m_ids) {
		EXPECT_TRUE(m_sim1->getPopulation()->m_original.at(id).isOnVacation());
	}
}

TEST_F(UnitTests__MR_SimulatorTest, destinationClusters) {
	// Test clusters of the target simulator, the travellers must be in the clusters
	for (unsigned int i = 0; i < m_sim2->getPopulation()->m_visitors.getAgenda().back()->size(); ++i) {
		auto& person = m_sim2->getPopulation()->m_visitors.getAgenda().back()->at(i);

		unsigned int work_index = person->getClusterId(ClusterType::Work);
		unsigned int prim_comm_index = person->getClusterId(ClusterType::PrimaryCommunity);
		unsigned int sec_comm_index = person->getClusterId(ClusterType::SecondaryCommunity);

		// Test whether the clusters exist
		ASSERT_NO_THROW(m_sim2->getClusters(ClusterType::Work).at(work_index));
		ASSERT_NO_THROW(m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index));
		ASSERT_NO_THROW(m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index));

		// Test every cluster on the presence of this person
		auto search_person = [&] (const pair<Simulator::PersonType*, bool> person_presence_pair) {return person.get() == person_presence_pair.first;};

		auto it = find_if(m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().begin(),
							m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().end(),
							search_person);

		EXPECT_NE(it, m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().end());

		it = find_if(m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().begin(),
						m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().end(),
						search_person);

		EXPECT_NE(it, m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().end());

		it = find_if(m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().begin(),
						m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().end(),
						search_person);

		EXPECT_NE(it, m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().end());
	}
}

}
