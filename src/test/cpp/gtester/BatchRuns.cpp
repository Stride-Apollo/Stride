/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of scenario tests running in batch mode.
 */

#include "pop/Population.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "util/etc.h"

#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>
#include <spdlog/spdlog.h>

#include <cmath>
#include <map>
#include <string>
#include <tuple>

using namespace std;
using namespace stride;
using namespace ::testing;

namespace Tests {

class Scenarios__BatchDemos: public ::testing::TestWithParam<tuple<string, unsigned int>> {
protected:

	// Default values
	const string         m_population_file             = "pop_oklahoma.csv";
	const double         m_r0                          = 3.0;
	const unsigned int   m_num_days                    = 30U;
	const unsigned int   m_rnm_seed                    = 2015U;
	const double         m_seedinm_rate                = 0.0001;
	const double         m_immunity_rate               = 0.0;
	const string         m_disease_confim_file         = "disease_influenza.xml";
	const string         m_output_prefix         	   = "test";
	const string         m_holidays_file               = "holidays_none.json";
	const unsigned int   m_num_participants_survey     = 10;
	const string         m_start_date                  = "2017-01-01";

	// Adapted values
	const double         m_seedinm_rate_adapted        = 0.0;
	const double         m_immunity_rate_adapted       = 0.999991;
	const string         m_disease_confim_file_adapted = "disease_measles.xml";
	const double         m_transmission_rate_measles   = 16U;
	const double         m_transmission_rate_maximum   = 100U;
	const string         m_population_file_flanders    = "pop_flanders.csv";
	const string         m_cluster_file_flanders       = "clusters_flanders.csv";

	static const map<string, unsigned int>   g_results;
};

const map<string, unsigned int> Scenarios__BatchDemos::g_results {
	make_pair("default", 70000),
	make_pair("seeding_rate", 0),
	make_pair("immunity_rate", 6),
	make_pair("measles", 135000),
	make_pair("maximum", 700000),
	make_pair("flanders", 10000)	// TODO reliable estimation (currently I based myself on pop_oklahoma.csv and assumed a linear correlation between population size and infected count)
};

TEST_P( Scenarios__BatchDemos, Run ) {
	// Prepare test configuration.
	// -----------------------------------------------------------------------------------------
	tuple<string, unsigned int> t(GetParam());
	const string test_tag = get<0>(t);
	const unsigned int num_threads = get<1>(t);
	// TODO_UNIPAR
	//omp_set_num_threads(num_threads);
	//omp_set_schedule(omp_sched_static,1);

	// Setup configuration.
	boost::property_tree::ptree pt_config;
	pt_config.put("run.rng_seed", m_rnm_seed);
	pt_config.put("run.r0", m_r0);
	pt_config.put("run.seeding_rate", m_seedinm_rate);
	pt_config.put("run.immunity_rate", m_immunity_rate);
	pt_config.put("run.population_file", m_population_file);
	pt_config.put("run.num_days", m_num_days);
	pt_config.put("run.output_prefix", m_output_prefix);
	pt_config.put("run.disease_config_file",m_disease_confim_file);
	pt_config.put("run.num_participants_survey",m_num_participants_survey);
	pt_config.put("run.start_date",m_start_date);
	pt_config.put("run.holidays_file",m_holidays_file);
	pt_config.put("run.age_contact_matrix_file","contact_matrix_average.xml");
	pt_config.put("run.log_level","None");
	bool track_index_case = false;

	// Override scenario settings.
	if (test_tag == "default") {
		// do nothing
	} else if (test_tag == "seeding_rate") {
		pt_config.put("run.seeding_rate", m_seedinm_rate_adapted);
	} else if (test_tag == "immunity_rate") {
		pt_config.put("run.seeding_rate", 1-m_immunity_rate_adapted);
		pt_config.put("run.immunity_rate", m_immunity_rate_adapted);
	} else if (test_tag == "measles") {
		pt_config.put("run.disease_config_file", m_disease_confim_file_adapted);
		pt_config.put("run.r0", m_transmission_rate_measles);
	} else if (test_tag == "maximum") {
		pt_config.put("run.r0", m_transmission_rate_maximum);
	} else if (test_tag == "flanders") {
		pt_config.put("run.population_file", m_population_file_flanders);
		pt_config.put("run.cluster_location_file", m_cluster_file_flanders);
	} else {
		FAIL() << "test_tag has an unexpected value: " << test_tag;
	}

	// initialize the logger.
	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", m_output_prefix + "_logfile",
			std::numeric_limits<size_t>::max(),  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// Release and close logger afterwards
	// We bind this to the current scope through 'defer', otherwise all subsequent tests
	// will fail if one test case throws an exception and doesn't do this.
	defer(spdlog::drop_all());

	// initialize the simulation.
	cout << "Building the simulator. " << endl;
	auto sim = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	cout << "Done building the simulator. " << endl << endl;

	// Run the simulation.
	const unsigned int num_days = pt_config.get<unsigned int>("run.num_days");
	for (unsigned int i = 0; i < num_days; i++) {
		sim->timeStep();
	}

	// Round up.
	const unsigned int num_cases = sim->getPopulation()->getInfectedCount();
	ASSERT_NEAR(num_cases, g_results.at(test_tag), 10000) << "!! CHANGED !!";
}

namespace {
#if UNIPAR_IMPL == UNIPAR_DUMMY
	unsigned int threads[] {1U};
#else
	unsigned int threads[] {1U, 4U, 8U};
#endif
}

INSTANTIATE_TEST_CASE_P(Run_default, Scenarios__BatchDemos,
        ::testing::Combine(::testing::Values(string("default")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_seeding_rate, Scenarios__BatchDemos,
        ::testing::Combine(::testing::Values(string("seeding_rate")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_immunity_rate, Scenarios__BatchDemos,
        ::testing::Combine(::testing::Values(string("immunity_rate")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_measles, Scenarios__BatchDemos,
        ::testing::Combine(::testing::Values(string("measles")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_maximum, Scenarios__BatchDemos,
        ::testing::Combine(::testing::Values(string("maximum")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_flanders, Scenarios__BatchDemos,
        ::testing::Combine(::testing::Values(string("flanders")), ::testing::ValuesIn(threads)));

}
