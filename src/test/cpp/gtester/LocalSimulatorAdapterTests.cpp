/**
 * @file
 * Implementation of tests for the Population Generator.
 */

#include <gtest/gtest.h>

#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "sim/AsyncSimulator.h"
#include "sim/LocalSimulatorAdapter.h"
#include "util/InstallDirs.h"
#include "util/TimeStamp.h"
#include "util/stdlib.h"
#include "util/async.h"

#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <cassert>
#include <string>
#include <vector>
#include <future>
#include <utility>



#include "output/CasesFile.h"
#include "output/PersonFile.h"
#include "output/SummaryFile.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "sim/AsyncSimulator.h"
#include "sim/LocalSimulatorAdapter.h"
#include "sim/Coordinator.h"
#include "util/ConfigInfo.h"
#include "util/InstallDirs.h"
#include "util/Stopwatch.h"
#include "util/TimeStamp.h"
#include "util/stdlib.h"

#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <cassert>


using namespace std;
using namespace stride;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;
using namespace output;
using namespace std::chrono;

namespace Tests {

class LocalSimulatorAdapterTest: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~LocalSimulatorAdapterTest() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// shared_ptr<Simulator> m_sim1;
	// shared_ptr<Simulator> m_sim2;
	// shared_ptr<Simulator> m_sim3;
	// unique_ptr<LocalSimulatorAdapter> m_sim_adapter_1;
	// unique_ptr<LocalSimulatorAdapter> m_sim_adapter_2;
	// unique_ptr<LocalSimulatorAdapter> m_sim_adapter_3;


};

TEST_F(LocalSimulatorAdapterTest, HappyDay_default) {
	// Tests which reflect the regular use
	// TODO remove sim 3 if I don't need it

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

	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", output_prefix + "_logfile",
												  std::numeric_limits<size_t>::max(),
												  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// Create simulator.
	Stopwatch<> total_clock("total_clock", true);
	auto sim = SimulatorBuilder::build(pt_config, num_threads, false);

	// No observers in C++. Logger was never intended as an observer per timestep.

	// MR test
	auto sim2 = SimulatorBuilder::build(pt_config, num_threads, false);
	auto sim3 = SimulatorBuilder::build(pt_config, num_threads, false);
	auto l1 = make_unique<LocalSimulatorAdapter>(sim.get());
	auto l2 = make_unique<LocalSimulatorAdapter>(sim2.get());
	auto l3 = make_unique<LocalSimulatorAdapter>(sim3.get());

	// Migrate 10 people for 10 days
	vector<unsigned int> id_s = l1->sendTravellers(10, 10, l2.get(), "Antwerp", "Airport 1");

	// Test if the people arrived in the destination simulator
	for (unsigned int i = 0; i < sim2->m_population->m_visitors.m_agenda.size(); ++i) {
		auto it = sim2->m_population->m_visitors.m_agenda.begin();
		auto block = (*(next(it, i))).get();
		if (i == 10) {
			ASSERT_EQ(block->size(), 10U);
			// These people can't be on vacation
			for (auto& person: *block) {
				EXPECT_FALSE(person.isOnVacation());
			}
		} else {
			EXPECT_EQ(block->size(), 0U);
		}
	}

	// Test if the people are absent in the home simulator
	for (auto id: id_s) {
		EXPECT_TRUE(sim->m_population->m_original.at(id).isOnVacation());
	}
		
	// Test clusters of the target simulator
	for (unsigned int i = 0; i < sim2->m_population->m_visitors.m_agenda.back()->size(); ++i) {
		auto& person = sim2->m_population->m_visitors.m_agenda.back()->at(i);

		unsigned int work_index = person.m_work_id;
		unsigned int prim_comm_index = person.m_primary_community_id;
		unsigned int sec_comm_index = person.m_secondary_community_id;

		// Test whether the clusters exist
		ASSERT_NO_THROW(sim2->m_work_clusters.at(work_index));
		ASSERT_NO_THROW(sim2->m_primary_community.at(prim_comm_index));
		ASSERT_NO_THROW(sim2->m_secondary_community.at(sec_comm_index));

		// Test every cluster on the presence of this person
		auto search_person = [&] (const pair<Simulator::PersonType*, bool> person_presence_pair) {return &person == person_presence_pair.first;};

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
			const Simulator::PersonType& person = sim2->m_population->m_visitors.getModifiableDay(10 - i)->at(j);
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

	// The agendas are empty now (or at least, it should be)
	EXPECT_EQ(l2->m_planner.m_agenda.size(), 0U);
	EXPECT_EQ(sim2->m_population->m_visitors.m_agenda.size(), 0U);

	// TODO test clusters of the target simulator (the travellers must have left)

	// Test whether the population in both manipulated sims is not on vacation
	for (const auto& person: sim->m_population->m_original) {
		EXPECT_FALSE(person.isOnVacation());
	}

	for (const auto& person: sim2->m_population->m_original) {
		EXPECT_FALSE(person.isOnVacation());
	}

	// Generate output files
	// Cases
	CasesFile cases_file(output_prefix);

	// Summary
	SummaryFile summary_file(output_prefix);
	summary_file.print(pt_config,
					   sim->getPopulation()->m_original.size(), sim->getPopulation()->getInfectedCount(),
					   duration_cast<milliseconds>(total_clock.get()).count(),
					   duration_cast<milliseconds>(total_clock.get()).count());

	if (pt_config.get<double>("run.generate_person_file") == 1) {
		PersonFile person_file(output_prefix);
		person_file.print(sim->getPopulation());
	}

	spdlog::drop_all();
}

TEST_F(LocalSimulatorAdapterTest, UnHappyDay_default) {

}

} //end-of-namespace-Tests
