/**
 * @file
 * Implementation of tests for the Population Generator.
 */

#include <gtest/gtest.h>

#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "sim/AsyncSimulator.h"
#include "sim/LocalSimulatorAdapter.h"
#include "util/async.h"
#include "util/ConfigInfo.h"
#include "util/InstallDirs.h"
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

class LocalSimulatorAdapterTest: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {
		
	}

protected:
	std::shared_ptr<Simulator> 				m_sim1;
	std::shared_ptr<Simulator> 				m_sim2;

	std::unique_ptr<LocalSimulatorAdapter> 	m_l1;
	std::unique_ptr<LocalSimulatorAdapter> 	m_l2;

protected:
	/// Destructor has to be virtual.
	virtual ~LocalSimulatorAdapterTest() {}

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
	}

	/// Tearing down the test fixture
	virtual void TearDown() {}
};


bool sameCluster(const Cluster& cluster1, const Cluster& cluster2) {
	// Test if 2 clusters are equal
	if (cluster1.getId() != cluster2.getId()) {
		return false;
	}
	
	if (cluster1.getClusterType() != cluster2.getClusterType()) {
		return false;
	}

	if (cluster1.getMembers().size() != cluster2.getMembers().size()) {
		return false;
	}

	for (auto& person_pair: cluster1.getMembers()) {
		Simulator::PersonType* person = person_pair.first;

		auto find_person = [&] (const pair<Simulator::PersonType*, bool>& pair) {return person == pair.first;};
		auto it = find_if(cluster2.getMembers().cbegin(), cluster2.getMembers().cend(), find_person);

		if (it == cluster2.getMembers().cend()) {
			return false;
		}
	}
	return true;
}

TEST_F(LocalSimulatorAdapterTest, HappyDay) {
	// Tests which reflect the regular use

	// Keep the original work, primary and secondary communities of simulator 2
	vector<Cluster> work_clusters = m_sim2->getClusters(ClusterType::Work);
	vector<Cluster> primary_community = m_sim2->getClusters(ClusterType::PrimaryCommunity);
	vector<Cluster> secondary_community = m_sim2->getClusters(ClusterType::SecondaryCommunity);

	// Keep the address of the first person, this is to avoid vector resizing causing a segmentation fault
	auto first_person = &(m_sim2->getPopulation()->m_original.at(0));

	// Migrate 10 people for 10 days
	vector<unsigned int> id_s = m_l1->sendTravellers(10, 10, m_l2.get(), "Antwerp", "ANR");

	// -----------------------------------------------------------------------------------------
	// Actual tests.
	// -----------------------------------------------------------------------------------------
	EXPECT_EQ(id_s.size(), 10U);
	ASSERT_EQ(&(m_sim2->getPopulation()->m_original.at(0)), first_person);

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
	for (auto id: id_s) {
		EXPECT_TRUE(m_sim1->getPopulation()->m_original.at(id).isOnVacation());
	}
		
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

		auto it = find_if(m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().begin(), m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().end(), search_person);
		EXPECT_NE(it, m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().end());

		it = find_if(m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().begin(), m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().end(), search_person);
		EXPECT_NE(it, m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().end());

		it = find_if(m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().begin(), m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().end(), search_person);
		EXPECT_NE(it, m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().end());
	}

	// Run the simulation.
	// Note: not 10 days, but 11, because the 10th day, these people will still be there, they depart (and instantly arrive) the day after
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

		for (unsigned int j = 0; j < m_l1->getPlanner().getAgenda().size(); ++j) {
			auto it = m_l2->getPlanner().getAgenda().begin();
			auto block = (*(next(it, j))).get();

			// Test whether they are present in the planner hosting region
			if (j == m_l2->getPlanner().getAgenda().size() - 1) {
				EXPECT_EQ(block->size(), 10U);
			} else {
				EXPECT_EQ(block->size(), 0U);
			}
		}
	}

	ASSERT_EQ(&(m_sim2->getPopulation()->m_original.at(0)), first_person);

	// The agendas are empty now (or at least, it should be)
	EXPECT_EQ(m_l2->getPlanner().getAgenda().size(), 0U);
	EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getAgenda().size(), 0U);

	// Test clusters of m_sim2, they must contain the same people as before
	for (uint i = 0; i < m_sim2->getClusters(ClusterType::Work).size(); ++i) {
		auto& cluster = m_sim2->getClusters(ClusterType::Work).at(i);
		EXPECT_TRUE(sameCluster(cluster, work_clusters.at(i)));
	}

	for (uint i = 0; i < m_sim2->getClusters(ClusterType::PrimaryCommunity).size(); ++i) {
		auto& cluster = m_sim2->getClusters(ClusterType::PrimaryCommunity).at(i);
		EXPECT_TRUE(sameCluster(cluster, primary_community.at(i)));
	}

	for (uint i = 0; i < m_sim2->getClusters(ClusterType::SecondaryCommunity).size(); ++i) {
		auto& cluster = m_sim2->getClusters(ClusterType::SecondaryCommunity).at(i);
		EXPECT_TRUE(sameCluster(cluster, secondary_community.at(i)));
	}

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

TEST_F(LocalSimulatorAdapterTest, ForceReturn) {
	m_l1->setId(1);
	m_l2->setId(2);

	// Keep the original work, primary and secondary communities of simulator 2
	vector<Cluster> work_clusters = m_sim2->getClusters(ClusterType::Work);
	vector<Cluster> primary_community = m_sim2->getClusters(ClusterType::PrimaryCommunity);
	vector<Cluster> secondary_community = m_sim2->getClusters(ClusterType::SecondaryCommunity);

	// Migrate 10 people for 10 days
	vector<unsigned int> id_s = m_l1->sendTravellers(10, 10, m_l2.get(), "Antwerp", "ANR");
	EXPECT_EQ(id_s.size(), 10U);

	// Test if the people are absent in the home simulator
	for (auto id: id_s) {
		EXPECT_TRUE(m_sim1->getPopulation()->m_original.at(id).isOnVacation());
	}

	// Get the data of the travellers
	EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getDay(10)->size(), 10U);
	vector<Simulator::PersonType> migrated_people_data;
	for (auto& person: *(m_sim2->getPopulation()->m_visitors.getDay(10))) {
		migrated_people_data.push_back(*person);
	}

	// Now force return them
	auto traveller_data = m_l2->forceReturn();
	EXPECT_EQ(traveller_data.size(), 10U);
	EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getAgenda().size(), 0U);
	EXPECT_EQ(m_l2->getPlanner().getAgenda().size(), 0U);
	EXPECT_EQ(m_sim1->getPopulation()->m_visitors.getAgenda().size(), 0U);
	EXPECT_EQ(m_l1->getPlanner().getAgenda().size(), 0U);

	// Test clusters of m_sim2, they must contain the same people as before
	for (uint i = 0; i < m_sim2->getClusters(ClusterType::Work).size(); ++i) {
		auto& cluster = m_sim2->getClusters(ClusterType::Work).at(i);
		EXPECT_TRUE(sameCluster(cluster, work_clusters.at(i)));
	}

	for (uint i = 0; i < m_sim2->getClusters(ClusterType::PrimaryCommunity).size(); ++i) {
		auto& cluster = m_sim2->getClusters(ClusterType::PrimaryCommunity).at(i);
		EXPECT_TRUE(sameCluster(cluster, primary_community.at(i)));
	}

	for (uint i = 0; i < m_sim2->getClusters(ClusterType::SecondaryCommunity).size(); ++i) {
		auto& cluster = m_sim2->getClusters(ClusterType::SecondaryCommunity).at(i);
		EXPECT_TRUE(sameCluster(cluster, secondary_community.at(i)));
	}

	// Test if the people are back in the home simulator
	for (auto id: id_s) {
		EXPECT_FALSE(m_sim1->getPopulation()->m_original.at(id).isOnVacation());
	}

	// Test the traveller data for correctness in the host simulator
	EXPECT_EQ(traveller_data.size(), migrated_people_data.size());
	for (auto data: traveller_data) {

		auto find_person = [&] (const Simulator::PersonType& person) {return person.getClusterId(ClusterType::Work) == data.m_destination_work_id
																				&& person.getClusterId(ClusterType::PrimaryCommunity) == data.m_destination_primary_id
																				&& person.getClusterId(ClusterType::SecondaryCommunity) == data.m_destination_secondary_id;};
		auto it = find_if(migrated_people_data.begin(), migrated_people_data.end(), find_person);

		EXPECT_NE(it, migrated_people_data.end());
		EXPECT_EQ(data.m_days_left, 10U);
		EXPECT_EQ(data.m_source_simulator, 1U);
		EXPECT_EQ(data.m_destination_simulator, 2U);
	}

	// Test the traveller data for correctness in the home simulator
	EXPECT_EQ(traveller_data.size(), id_s.size());
	for (auto data: traveller_data) {

		auto find_person = [&] (const uint& id) {return m_sim1->getPopulation()->m_original.at(id).getAge() == data.m_home_age
															&& m_sim1->getPopulation()->m_original.at(id).getId() == data.m_home_id;};
		auto it = find_if(id_s.begin(), id_s.end(), find_person);

		EXPECT_NE(it, id_s.end());
	}
}

TEST_F(LocalSimulatorAdapterTest, ForceHost) {
	// Migrate 1 person for 10 days
	vector<unsigned int> id_s = m_l1->sendTravellers(1, 10, m_l2.get(), "Antwerp", "ANR");
	EXPECT_EQ(id_s.size(), 1U);

	// Get the data of the traveller
	EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getDay(10)->size(), 1U);
	vector<Simulator::PersonType> migrated_people_data;
	for (auto& person: *(m_sim2->getPopulation()->m_visitors.getDay(10))) {
		migrated_people_data.push_back(*person);
	}

	// Now force return him
	auto traveller_data = m_l2->forceReturn();

	// Find the person
	// Now force him back to his destination
	m_l1->forceSend(traveller_data.at(0), m_l2.get());

	// // Test if the person arrived in the destination simulator
	for (unsigned int i = 0; i < m_sim2->getPopulation()->m_visitors.getAgenda().size(); ++i) {
		auto it = m_sim2->getPopulation()->m_visitors.getAgenda().begin();
		auto block = (*(next(it, i))).get();
		if (i == 10) {
			ASSERT_EQ(block->size(), 1U);
			// These people can't be on vacation
			for (auto& person: *block) {
				EXPECT_FALSE(person->isOnVacation());
			}
		} else {
			EXPECT_EQ(block->size(), 0U);
		}
	}

	// Test if the person is absent in the home simulator
	EXPECT_TRUE(m_sim1->getPopulation()->m_original.at(id_s.at(0)).isOnVacation());
		
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

		auto it = find_if(m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().begin(), m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().end(), search_person);
		EXPECT_NE(it, m_sim2->getClusters(ClusterType::Work).at(work_index).getMembers().end());

		it = find_if(m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().begin(), m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().end(), search_person);
		EXPECT_NE(it, m_sim2->getClusters(ClusterType::PrimaryCommunity).at(prim_comm_index).getMembers().end());

		it = find_if(m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().begin(), m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().end(), search_person);
		EXPECT_NE(it, m_sim2->getClusters(ClusterType::SecondaryCommunity).at(sec_comm_index).getMembers().end());
	}
}

TEST_F(LocalSimulatorAdapterTest, getTravellerData) {
	// Migrate 1 person for 10 days
	vector<unsigned int> id_s = m_l1->sendTravellers(1, 10, m_l2.get(), "Antwerp", "ANR");
	EXPECT_EQ(id_s.size(), 1U);

	// Get the data of the traveller
	EXPECT_EQ(m_sim2->getPopulation()->m_visitors.getDay(10)->size(), 1U);
	vector<Simulator::PersonType> migrated_people_data;
	for (auto& person: *(m_sim2->getPopulation()->m_visitors.getDay(10))) {
		migrated_people_data.push_back(*person);
	}

	// This data must be the same as getTravellerData
	auto traveller_data = m_l2->getTravellerData();
	ASSERT_EQ(traveller_data.size(), migrated_people_data.size());
	ASSERT_EQ(traveller_data.size(), id_s.size());

	for (uint i = 0; i < traveller_data.size(); ++i) {
		auto& data = traveller_data.at(i);
		auto& migrated_person = migrated_people_data.at(i);
		auto& home_person = m_sim1->getPopulation()->m_original.at(id_s[i]);

		EXPECT_EQ(data.m_home_id, home_person.getId());
		EXPECT_EQ(data.m_home_age, migrated_person.getAge());
		EXPECT_EQ(data.m_home_age, home_person.getAge());
		
		EXPECT_EQ(migrated_person.getClusterId(ClusterType::Work), data.m_destination_work_id);
		EXPECT_EQ(migrated_person.getClusterId(ClusterType::PrimaryCommunity), data.m_destination_primary_id);
		EXPECT_EQ(migrated_person.getClusterId(ClusterType::SecondaryCommunity), data.m_destination_secondary_id);
	}
}

}
