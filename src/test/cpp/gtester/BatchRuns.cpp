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
	const double         m_seeding_rate                = 0.0001;
	const double         m_immunity_rate               = 0.0;
	const string         m_disease_config_file         = "disease_influenza.xml";
	const string         m_name			         	   = "test";
	const string         m_holidays_file               = "holidays_none.json";
	const unsigned int   m_num_participants_survey     = 10;
	const string         m_start_date                  = "2017-01-01";

	// Adapted values
	const double         m_seeding_rate_adapted        = 0.0;
	const double         m_immunity_rate_adapted       = 0.999991;
	const string         m_disease_config_file_adapted = "disease_measles.xml";
	const double         m_transmission_rate_measles   = 16U;
	const double         m_transmission_rate_maximum   = 100U;

	static const map<string, unsigned int>   g_results;
};

const map<string, unsigned int> Scenarios__BatchDemos::g_results {
	make_pair("default", 80000),
	make_pair("seeding_rate", 0),
	make_pair("immunity_rate", 6),
	make_pair("measles", 135000),
	make_pair("maximum", 700000),
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
	pt_config.put("run.regions.region.rng_seed", m_rnm_seed);
	pt_config.put("run.r0", m_r0);
	pt_config.put("run.disease.seeding_rate", m_seeding_rate);
	pt_config.put("run.disease.immunity_rate", m_immunity_rate);
	pt_config.put("run.regions.region.raw_population", m_population_file); 
	pt_config.put("run.num_days", m_num_days);
	pt_config.put("run.<xmlattr>.name", m_name);
	pt_config.put("run.disease.config", m_disease_config_file);
	pt_config.put("run.outputs.participants_survey.<xmlattr>.num", m_num_participants_survey);
	pt_config.put("run.start_date", m_start_date);
	pt_config.put("run.holidays", m_holidays_file);
	pt_config.put("run.age_contact_matrix_file","contact_matrix_average.xml");
	pt_config.put("run.outputs.log.<xmlattr>.log_level", "None");
	bool track_index_case = false;

	// Override scenario settings.
	if (test_tag == "default") {
		// do nothing
	} else if (test_tag == "seeding_rate") {
		pt_config.put("run.disease.seeding_rate", m_seeding_rate_adapted);
	} else if (test_tag == "immunity_rate") {
		pt_config.put("run.disease.seeding_rate", 1-m_immunity_rate_adapted);
		pt_config.put("run.disease.immunity_rate", m_immunity_rate_adapted);
	} else if (test_tag == "measles") {
		pt_config.put("run.disease.config", m_disease_config_file_adapted);
		pt_config.put("run.r0", m_transmission_rate_measles);
	} else if (test_tag == "maximum") {
		pt_config.put("run.r0", m_transmission_rate_maximum);
	} else {
		FAIL() << "test_tag has an unexpected value: " << test_tag;
	}

	// initialize the logger.
	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", m_name + "_logfile",
			std::numeric_limits<size_t>::max(),  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// Release and close logger afterwards
	// We bind this to the current scope through 'defer', otherwise all subsequent tests
	// will fail if one test case throws an exception and doesn't do this.
	defer(spdlog::drop_all());

	// initialize the simulation.
	auto sim = SimulatorBuilder::build(pt_config);

	// Run the simulation.
	const unsigned int num_days = pt_config.get<unsigned int>("run.num_days");
	for (unsigned int i = 0; i < num_days; i++) {
		sim->timeStep();
	}

	// Round up.
	const unsigned int num_cases = sim->getPopulation()->getInfectedCount();
	#if UNIPAR_IMPL == UNIPAR_DUMMY
		ASSERT_NEAR(num_cases, g_results.at(test_tag), 10000) << "!! CHANGED !!";
	#else
		ASSERT_NEAR(num_cases, g_results.at(test_tag), 30000) << "!! CHANGED !!";
	#endif
}

namespace {
#if UNIPAR_IMPL == UNIPAR_DUMMY
	unsigned int threads[] {1U};
#else
	unsigned int threads[] {1U, 4U};
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
}
