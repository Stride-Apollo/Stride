/*
\ *  This is free software: you can redistribute it and/or modify it
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
#include <omp.h>
#include <spdlog/spdlog.h>

#include <cmath>
#include <map>
#include <string>
#include <tuple>

using namespace std;
using namespace stride;
using namespace ::testing;

namespace Tests {

class BatchDemos: public ::testing::TestWithParam<tuple<string, unsigned int>> {
protected:
	static const string            g_population_file;
	static const double            g_r0;
	static const unsigned int      g_num_days;
	static const unsigned int      g_rng_seed;
	static const double            g_seeding_rate;
	static const double            g_immunity_rate;
	static const string            g_disease_config_file;
	static const string            g_output_prefix;
	static const string            g_holidays_file;
	static const unsigned int      g_num_participants_survey;
	static const string            g_start_date;

	static const unsigned int      g_rng_seed_adapted;
	static const double            g_seeding_rate_adapted;
	static const double            g_immunity_rate_adapted;
	static const string            g_disease_config_file_adapted;
	static const double            g_transmission_rate_measles;
	static const double            g_transmission_rate_maximum;
	static const string            g_population_file_flanders;
	static const string            g_cluster_file_flanders;

	static const map<string, unsigned int>   g_results;
};

// Default values
const string         BatchDemos::g_population_file             = "pop_oklahoma.csv";
const double         BatchDemos::g_r0                          = 3.0;
const unsigned int   BatchDemos::g_num_days                    = 30U;
const unsigned int   BatchDemos::g_rng_seed                    = 2015U;
const double         BatchDemos::g_seeding_rate                = 0.0001;
const double         BatchDemos::g_immunity_rate               = 0.0;
const string         BatchDemos::g_disease_config_file         = "disease_influenza.xml";
const string         BatchDemos::g_output_prefix         	   = "test";
const string         BatchDemos::g_holidays_file               = "holidays_none.json";
const unsigned int   BatchDemos::g_num_participants_survey     = 10;
const string         BatchDemos::g_start_date                  = "2017-01-01";

// Adapted values
const double         BatchDemos::g_seeding_rate_adapted        = 0.0;
const double         BatchDemos::g_immunity_rate_adapted       = 0.999991;
const string         BatchDemos::g_disease_config_file_adapted = "disease_measles.xml";
const double         BatchDemos::g_transmission_rate_measles   = 16U;
const double         BatchDemos::g_transmission_rate_maximum   = 100U;
const string         BatchDemos::g_population_file_flanders    = "pop_flanders.csv";
const string         BatchDemos::g_cluster_file_flanders       = "clusters_flanders.csv";

const map<string, unsigned int> BatchDemos::g_results {
	make_pair("default", 70000),
	make_pair("seeding_rate",0),
	make_pair("immunity_rate",6),
	make_pair("measles",135000),
	make_pair("maximum",700000),
	make_pair("flanders",10000)	// TODO reliable estimation (currently I based myself on pop_oklahoma.csv and assumed a linear correlation between population size and infected count)
};

TEST_P( BatchDemos, Run ) {
	// Prepare test configuration.
	tuple<string, unsigned int> t(GetParam());
	const string test_tag = get<0>(t);
	const unsigned int num_threads = get<1>(t);
	// TODO_UNIPAR
	//omp_set_num_threads(num_threads);
	//omp_set_schedule(omp_sched_static,1);

	// Setup configuration.
	boost::property_tree::ptree pt_config;
	pt_config.put("run.rng_seed", g_rng_seed);
	pt_config.put("run.r0", g_r0);
	pt_config.put("run.seeding_rate", g_seeding_rate);
	pt_config.put("run.immunity_rate", g_immunity_rate);
	pt_config.put("run.population_file", g_population_file);
	pt_config.put("run.num_days", g_num_days);
	pt_config.put("run.output_prefix", g_output_prefix);
	pt_config.put("run.disease_config_file",g_disease_config_file);
	pt_config.put("run.num_participants_survey",g_num_participants_survey);
	pt_config.put("run.start_date",g_start_date);
	pt_config.put("run.holidays_file",g_holidays_file);
	pt_config.put("run.age_contact_matrix_file","contact_matrix_average.xml");
	pt_config.put("run.log_level","None");
	bool track_index_case = false;

	// Override scenario settings.
	if (test_tag == "default") {
		// do nothing
	} else if (test_tag == "seeding_rate") {
		pt_config.put("run.seeding_rate", g_seeding_rate_adapted);
	} else if (test_tag == "immunity_rate") {
		pt_config.put("run.seeding_rate", 1-g_immunity_rate_adapted);
		pt_config.put("run.immunity_rate", g_immunity_rate_adapted);
	} else if (test_tag == "measles") {
		pt_config.put("run.disease_config_file", g_disease_config_file_adapted);
		pt_config.put("run.r0", g_transmission_rate_measles);
	} else if (test_tag == "maximum") {
		pt_config.put("run.r0", g_transmission_rate_maximum);
	} else {
		FAIL() << "test_tag has an unexpected value: " << test_tag;
	}

	// initialize the logger.
	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", g_output_prefix + "_logfile",
			std::numeric_limits<size_t>::max(),  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// Release and close logger afterwards
	// We bind this the current scope through 'defer', otherwise all subsequent tests
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
#ifdef _OPENMP
	unsigned int threads[] {1U, 4U, 8U};
#else
	unsigned int threads[] { 1U };
#endif
}

INSTANTIATE_TEST_CASE_P(Run_default, BatchDemos,
        ::testing::Combine(::testing::Values(string("default")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_seeding_rate, BatchDemos,
        ::testing::Combine(::testing::Values(string("seeding_rate")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_immunity_rate, BatchDemos,
        ::testing::Combine(::testing::Values(string("immunity_rate")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_measles, BatchDemos,
        ::testing::Combine(::testing::Values(string("measles")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_maximum, BatchDemos,
        ::testing::Combine(::testing::Values(string("maximum")), ::testing::ValuesIn(threads)));

INSTANTIATE_TEST_CASE_P(Run_flanders, BatchDemos,
        ::testing::Combine(::testing::Values(string("flanders")), ::testing::ValuesIn(threads)));

}
