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

class UnitTests__AsyncSimulatorTest: public ::testing::Test {
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
	virtual ~UnitTests__AsyncSimulatorTest() {}

	/// Set up for the test fixture
	virtual void SetUp() {
		ptree pt_config;
		const auto file_path = InstallDirs::getDataDir() /= string("../config/run_flanders.xml");

		std::ifstream my_file;
		my_file.open(file_path.string());

		if (my_file.bad()) {
			throw runtime_error(string(__func__)
								+ ">Config file " + file_path.string() + " not present. Aborting.");
		}
		read_xml(file_path.string(), pt_config);

		// TODO Unipar
		unsigned int num_threads = 1;

		// Set output path prefix.
		string output_prefix = "";

		// Additional run configurations.
		if (pt_config.get_optional<bool>("run.num_participants_survey") == false) {
			pt_config.put("run.num_participants_survey", 1);
		}

		// Create simulators
		m_sim1 = SimulatorBuilder::build(pt_config, num_threads, false);
		m_sim2 = SimulatorBuilder::build(pt_config, num_threads, false);
		m_l1 = make_unique<LocalSimulatorAdapter>(m_sim1.get());
		m_l2 = make_unique<LocalSimulatorAdapter>(m_sim2.get());

		m_l1->setId("1");
		m_l2->setId("2");

		map<string, AsyncSimulator*> sender_map1;
		sender_map1["2"] = m_l2.get();

		map<string, AsyncSimulator*> sender_map2;
		sender_map2["1"] = m_l1.get();

		m_l1->setCommunicationMap(sender_map1);
		m_l2->setCommunicationMap(sender_map2);

		// Migrate 10 people for 10 days
		m_l1->sendNewTravellers(10, 10, "2", "Antwerp", "ANR");

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

TEST_F(UnitTests__AsyncSimulatorTest, returnHome) {
	for (unsigned int i = 0; i < 11; i++) {
		// Test whether they are present in the population of the hosting region
		EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getDay(10 - i)->size(), 10U);
		for (unsigned int j = 0; j < m_sim2->getPopulation()->m_visitors.getDay(10 - i)->size(); ++j) {
			// They shouldn't be on vacation
			const Simulator::PersonType& person = *(m_sim2->getPopulation()->m_visitors.getDay(10 - i)->at(j));
			EXPECT_FALSE(person.isOnVacation());
		}

		vector<future<bool>> fut_results;
		fut_results.push_back(m_l1->timeStep());
		fut_results.push_back(m_l2->timeStep());
		future_pool(fut_results);

		m_l1->returnForeignTravellers();
		m_l2->returnForeignTravellers();

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

TEST_F(UnitTests__AsyncSimulatorTest, peopleMoved) {
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

TEST_F(UnitTests__AsyncSimulatorTest, destinationClusters) {
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
