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
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Actually run the simulator.
 */

# include "run_stride2.h"

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

namespace stride {

using namespace output;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;
using namespace std::chrono;

/// Run the stride simulator.
void run_stride2(bool track_index_case, const string& config_file_name) {
	// C++ example of how a Python main loop would look

	// Configuration
	// with open(config_file_name) as f:
	//     config = json.load(f)
	ptree pt_config;
	const auto file_path = canonical(system_complete(config_file_name));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ ">Config file " + file_path.string() + " not present. Aborting.");
	}
	read_xml(file_path.string(), pt_config);
	cout << "Configuration file:  " << file_path.string() << endl;

	// OpenMP
	unsigned int num_threads;
	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	if (ConfigInfo::haveOpenMP()) {
		cout << "Using OpenMP threads:  " << num_threads << endl;
	} else {
		cout << "Not using OpenMP threads." << endl;
	}

	// Set output path prefix.
	string output_prefix = "";

	// Additional run configurations.
	if (pt_config.get_optional<bool>("run.num_participants_survey") == false) {
		pt_config.put("run.num_participants_survey", 1);
	}

	// Track index case setting.
	cout << "Setting for track_index_case:  " << boolalpha << track_index_case << endl;

	// Create logger
	// Transmissions:     [TRANSMISSION] <infecterID> <infectedID> <clusterID> <day>
	// General contacts:  [CNT] <person1ID> <person1AGE> <person2AGE>  <at_home> <at_work> <at_school> <at_other>
	// Note, the logger here is kinda like a global variable, but checked at runtime :(
	// This is somewhat common practice in loggers, but I'd still prefer an alternative approach
	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", output_prefix + "_logfile",
												  std::numeric_limits<size_t>::max(),
												  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// Create simulator.
	Stopwatch<> total_clock("total_clock", true);
	cout << "Building the simulators." << endl;
	auto sim = SimulatorBuilder::build(pt_config, num_threads, track_index_case);

	// No observers in C++. Logger was never intended as an observer per timestep.

	// MR test
	auto sim2 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	auto sim3 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	auto sim4 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	auto sim5 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	auto sim6 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	auto sim7 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	auto sim8 = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	cout << "Done building the simulators." << endl << endl;
	auto l1 = make_unique<LocalSimulatorAdapter>(sim.get());
	auto l2 = make_unique<LocalSimulatorAdapter>(sim2.get());
	auto l3 = make_unique<LocalSimulatorAdapter>(sim3.get());
	auto l4 = make_unique<LocalSimulatorAdapter>(sim4.get());
	auto l5 = make_unique<LocalSimulatorAdapter>(sim5.get());
	auto l6 = make_unique<LocalSimulatorAdapter>(sim6.get());
	auto l7 = make_unique<LocalSimulatorAdapter>(sim7.get());
	auto l8 = make_unique<LocalSimulatorAdapter>(sim8.get());

	// Build the coordinator
	if (InstallDirs::getDataDir().empty()) {
		throw runtime_error(string(__func__) + "> Data directory not present! Aborting.");
	}

	// Check if the node exists, allow XML's without the flight schedule
	string traveller_file = "";
	if(pt_config.get_child_optional("run.traveller_file")) {
		traveller_file = pt_config.get<string>("run.traveller_file");
		traveller_file = (InstallDirs::getDataDir() /= traveller_file).string();
	}

	vector<LocalSimulatorAdapter*> sim_vector = {l1.get(), l2.get(), l3.get(), l4.get(), l5.get(), l6.get(), l7.get(), l8.get()};
	Coordinator coord(sim_vector, traveller_file);

	// Run the simulation.
	const unsigned int num_days = pt_config.get<unsigned int>("run.num_days");
	vector<unsigned int> cases(num_days);
	for (unsigned int i = 0; i < num_days; i++) {
		cout << "Simulating day: " << setw(5) << i;
		//sim->timeStep();
		coord.timeStep();
		cout << "     Done, infected counts: ";
		cases[i] = sim->getPopulation()->getInfectedCount();

		cout << setw(10) << cases[i];
		for (uint i = 1; i < sim_vector.size(); ++i) {
			cout << setw(10) << sim_vector.at(i)->m_sim->getPopulation()->getInfectedCount();
		}
		cout << endl;
	}

	// Generate output files
	// Cases
	CasesFile cases_file(output_prefix);
	cases_file.print(cases);

	// Summary
	SummaryFile summary_file(output_prefix);
	summary_file.print(pt_config,
					   sim->getPopulation()->m_original.size(), sim->getPopulation()->getInfectedCount(),
					   duration_cast<milliseconds>(total_clock.get()).count(),
					   duration_cast<milliseconds>(total_clock.get()).count());

	// Persons ???
	if (pt_config.get<double>("run.generate_person_file") == 1) {
		PersonFile person_file(output_prefix);
		person_file.print(sim->getPopulation());
	}

	// print final message to command line.
	cout << endl << endl;
	cout << "  total time: " << total_clock.toString() << endl << endl;
}

}
