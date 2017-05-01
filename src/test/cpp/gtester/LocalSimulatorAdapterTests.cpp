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
#include "util/stdlib.h"
#include "core/Cluster.h"

#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
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

bool sameCluster(const Cluster& cluster1, const Cluster& cluster2) {
	if (cluster1.m_cluster_id != cluster2.m_cluster_id) {
		return false;
	}
	
	if (cluster1.m_cluster_type != cluster2.m_cluster_type) {
		return false;
	}

	if (cluster1.m_members.size() != cluster2.m_members.size()) {
		return false;
	}

	for (auto& person_pair: cluster1.m_members) {
		Simulator::PersonType* person = person_pair.first;

		auto find_person = [&] (const pair<Simulator::PersonType*, bool>& pair) {return person == pair.first;};
		auto it = find_if(cluster2.m_members.begin(), cluster2.m_members.end(), find_person);

		if (it == cluster2.m_members.end()) {
			return false;
		}
	}
	return true;
}

TEST(LocalSimulatorAdapterTest, HappyDay_default) {
	// Tests which reflect the regular use

	// -----------------------------------------------------------------------------------------
	// Prepare test configuration.
	// -----------------------------------------------------------------------------------------

	ptree pt_config;
	const auto file_path = canonical(system_complete("../config/run_flanders.xml"));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ ">Config file " + file_path.string() + " not present. Aborting.");
	}
	read_xml(file_path.string(), pt_config);

	// OpenMP
	unsigned int num_threads;
	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}

	// Set output path prefix.
	string output_prefix = "";

	// Additional run configurations.
	if (pt_config.get_optional<bool>("run.num_participants_survey") == false) {
		pt_config.put("run.num_participants_survey", 1);
	}

	// Create simulators
	auto sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto sim2 = SimulatorBuilder::build(pt_config, num_threads, false);
	auto sim3 = SimulatorBuilder::build(pt_config, num_threads, false);
	auto l1 = make_unique<LocalSimulatorAdapter>(sim.get());
	auto l2 = make_unique<LocalSimulatorAdapter>(sim2.get());
	auto l3 = make_unique<LocalSimulatorAdapter>(sim3.get());

	// Keep the original work, primary and secondary communities of simulator 2
	vector<Cluster> work_clusters = sim2->m_work_clusters;
	vector<Cluster> primary_community = sim2->m_primary_community;
	vector<Cluster> secondary_community = sim2->m_secondary_community;

	// Keep the address of the first person, this is to avoid vector resizing causing a segmentation fault
	auto first_person = &(sim2->m_population->m_original.at(0));

	// Migrate 10 people for 10 days
	vector<unsigned int> id_s = l1->sendTravellers(10, 10, l2.get(), "Antwerp", "ANR");
	EXPECT_EQ(id_s.size(), 10U);


	// -----------------------------------------------------------------------------------------
	// Actual tests.
	// -----------------------------------------------------------------------------------------
	ASSERT_EQ(&(sim2->m_population->m_original.at(0)), first_person);

	// Test if the people arrived in the destination simulator
	for (unsigned int i = 0; i < sim2->m_population->m_visitors.m_agenda.size(); ++i) {
		auto it = sim2->m_population->m_visitors.m_agenda.begin();
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
		EXPECT_TRUE(sim->m_population->m_original.at(id).isOnVacation());
	}
		
	// Test clusters of the target simulator, the travellers must be in the clusters
	for (unsigned int i = 0; i < sim2->m_population->m_visitors.m_agenda.back()->size(); ++i) {
		auto& person = sim2->m_population->m_visitors.m_agenda.back()->at(i);

		unsigned int work_index = person->m_work_id;
		unsigned int prim_comm_index = person->m_primary_community_id;
		unsigned int sec_comm_index = person->m_secondary_community_id;

		// Test whether the clusters exist
		ASSERT_NO_THROW(sim2->m_work_clusters.at(work_index));
		ASSERT_NO_THROW(sim2->m_primary_community.at(prim_comm_index));
		ASSERT_NO_THROW(sim2->m_secondary_community.at(sec_comm_index));

		// Test every cluster on the presence of this person
		auto search_person = [&] (const pair<Simulator::PersonType*, bool> person_presence_pair) {return person.get() == person_presence_pair.first;};

		auto it = find_if(sim2->m_work_clusters.at(work_index).m_members.begin(), sim2->m_work_clusters.at(work_index).m_members.end(), search_person);
		EXPECT_NE(it, sim2->m_work_clusters.at(work_index).m_members.end());

		it = find_if(sim2->m_primary_community.at(prim_comm_index).m_members.begin(), sim2->m_primary_community.at(prim_comm_index).m_members.end(), search_person);
		EXPECT_NE(it, sim2->m_primary_community.at(prim_comm_index).m_members.end());

		it = find_if(sim2->m_secondary_community.at(sec_comm_index).m_members.begin(), sim2->m_secondary_community.at(sec_comm_index).m_members.end(), search_person);
		EXPECT_NE(it, sim2->m_secondary_community.at(sec_comm_index).m_members.end());
	}

	// Run the simulation.
	// Note: not 10 days, but 11, because the 10th day, these people will still be there, they depart (and instantly arrive) the day after
	for (unsigned int i = 0; i < 11; i++) {

		// Test whether they are present in the population of the hosting region
		EXPECT_EQ(sim2->m_population->m_visitors.getDay(10 - i)->size(), 10U);
		for (unsigned int j = 0; j < sim2->m_population->m_visitors.getDay(10 - i)->size(); ++j) {
			// They shouldn't be on vacation
			const Simulator::PersonType& person = *(sim2->m_population->m_visitors.getModifiableDay(10 - i)->at(j));
			EXPECT_FALSE(person.isOnVacation());
		}

		vector<future<bool>> fut_results;
		fut_results.push_back(l1->timeStep());
		fut_results.push_back(l2->timeStep());
		fut_results.push_back(l3->timeStep());
		future_pool(fut_results);

		for (unsigned int j = 0; j < l1->m_planner.m_agenda.size(); ++j) {
			auto it = l2->m_planner.m_agenda.begin();
			auto block = (*(next(it, j))).get();

			// Test whether they are present in the planner hosting region
			if (j == l2->m_planner.m_agenda.size() - 1) {
				EXPECT_EQ(block->size(), 10U);
			} else {
				EXPECT_EQ(block->size(), 0U);
			}
		}
	}

	ASSERT_EQ(&(sim2->m_population->m_original.at(0)), first_person);

	// The agendas are empty now (or at least, it should be)
	EXPECT_EQ(l2->m_planner.m_agenda.size(), 0U);
	EXPECT_EQ(sim2->m_population->m_visitors.m_agenda.size(), 0U);

	// Test clusters of sim2, they must contain the same people as before
	for (uint i = 0; i < sim2->m_work_clusters.size(); ++i) {
		auto& cluster = sim2->m_work_clusters.at(i);
		EXPECT_TRUE(sameCluster(cluster, work_clusters.at(i)));
	}

	for (uint i = 0; i < sim2->m_primary_community.size(); ++i) {
		auto& cluster = sim2->m_primary_community.at(i);
		EXPECT_TRUE(sameCluster(cluster, primary_community.at(i)));
	}

	for (uint i = 0; i < sim2->m_secondary_community.size(); ++i) {
		auto& cluster = sim2->m_secondary_community.at(i);
		EXPECT_TRUE(sameCluster(cluster, secondary_community.at(i)));
	}

	// Test whether the population in both manipulated sims is not on vacation

	for (uint i = 0; i < sim->m_population->m_original.size(); ++i) {
		auto& person = sim->m_population->m_original.at(i);
		EXPECT_FALSE(person.isOnVacation());
	}

	for (uint i = 0; i < sim2->m_population->m_original.size(); ++i) {
		auto& person = sim2->m_population->m_original.at(i);
		EXPECT_FALSE(person.isOnVacation());
	}
}

TEST(LocalSimulatorAdapterTest, ForceReturn_default) {
	ptree pt_config;
	const auto file_path = canonical(system_complete("../config/run_flanders.xml"));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ ">Config file " + file_path.string() + " not present. Aborting.");
	}
	read_xml(file_path.string(), pt_config);

	// OpenMP
	unsigned int num_threads;
	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}

	// Set output path prefix.
	string output_prefix = "";

	// Additional run configurations.
	if (pt_config.get_optional<bool>("run.num_participants_survey") == false) {
		pt_config.put("run.num_participants_survey", 1);
	}

	// Create simulators
	auto sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto sim2 = SimulatorBuilder::build(pt_config, num_threads, false);
	auto l1 = make_unique<LocalSimulatorAdapter>(sim.get());
	auto l2 = make_unique<LocalSimulatorAdapter>(sim2.get());

	// Migrate 10 people for 10 days
	vector<unsigned int> id_s = l1->sendTravellers(10, 10, l2.get(), "Antwerp", "ANR");
	EXPECT_EQ(id_s.size(), 10U);

	// Test if the people are absent in the home simulator
	for (auto id: id_s) {
		EXPECT_TRUE(sim->m_population->m_original.at(id).isOnVacation());
	}

	// Get the data of the travellers
	EXPECT_EQ(sim2->m_population->m_visitors.getDay(10)->size(), 10);
	vector<Simulator::PersonType> migrated_people_data;
	for (auto& person: *(sim2->m_population->m_visitors.getDay(10))) {
		migrated_people_data.push_back(*person);
	}

	// Now force return them
	auto traveller_data = l2->forceReturn();
	EXPECT_EQ(traveller_data.size(), 10U);

	// Test if the people are back in the home simulator
	for (auto id: id_s) {
		EXPECT_FALSE(sim->m_population->m_original.at(id).isOnVacation());
	}

	// Test the traveller data for correctness in the host simulator
	EXPECT_EQ(traveller_data.size(), migrated_people_data.size());
	for (auto data: traveller_data) {

		auto find_person = [&] (const Simulator::PersonType& person) {return person.getClusterId(ClusterType::Work) == data.m_destination_work_id
																				&& person.getClusterId(ClusterType::PrimaryCommunity) == data.m_destination_primary_id
																				&& person.getClusterId(ClusterType::SecondaryCommunity) == data.m_destination_secondary_id;};
		auto it = find_if(migrated_people_data.begin(), migrated_people_data.end(), find_person);

		EXPECT_NE(it, migrated_people_data.end());
	}

	// Test the traveller data for correctness in the home simulator
	EXPECT_EQ(traveller_data.size(), id_s.size());
	for (auto data: traveller_data) {

		auto find_person = [&] (const uint& id) {return sim->m_population->m_original.at(id).getAge() == data.m_home_age
															&& sim->m_population->m_original.at(id).getId() == data.m_home_id;};
		auto it = find_if(id_s.begin(), id_s.end(), find_person);

		EXPECT_NE(it, id_s.end());
	}
}

} //end-of-namespace-Tests
