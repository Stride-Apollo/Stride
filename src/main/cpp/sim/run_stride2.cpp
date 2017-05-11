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

	// Configuration
	ptree pt_config;
	const auto file_path = canonical(system_complete(config_file_name));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ ">Coordinator config file " + file_path.string() + " not present. Aborting.");
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

	// TODO: num_participants_survey

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

	// Count the amount of simulators
	unsigned int simulator_amount = 0;
	auto sim_config = pt_config.get_child("coordinator");

	for (auto it = sim_config.begin(); it != sim_config.end(); ++it) {
		if (it->first == "sim") {
			++simulator_amount;
		}
	}

	vector<shared_ptr<Simulator>> simulators;
	vector<shared_ptr<LocalSimulatorAdapter> > simulator_adapters;
	vector<LocalSimulatorAdapter*> simulator_adapters_raw;

	simulators.reserve(simulator_amount);
	simulator_adapters.reserve(simulator_amount);
	simulator_adapters_raw.reserve(simulator_amount);

	// Calculate the amount of threads per simulator
	cout << "Total simulators: " << simulator_amount << endl;
	cout << "Total threads: " << (num_threads > simulator_amount ? num_threads : simulator_amount) << endl;
	unsigned int threads_per_sim = floor(num_threads / simulator_amount) > 0 ? floor(num_threads / simulator_amount) : 1;
	int leftover_threads = num_threads - threads_per_sim * simulator_amount;

	// Build the simulators
	for (auto it = sim_config.begin(); it != sim_config.end(); ++it) {
		if (it->first == "sim") {
			string filename = "./config/" + it->second.data();

			auto sim = SimulatorBuilder::build(filename, threads_per_sim + (leftover_threads > 0), track_index_case);
			simulators.push_back(sim);

			auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());
			simulator_adapters.push_back(local_sim);

			simulator_adapters_raw.push_back(local_sim.get());

			cout << "Built simulator using " << threads_per_sim + (leftover_threads > 0) << " thread(s).\n";
			--leftover_threads;
		}else {
			// TODO exception
		}
	}
	cout << "Done building the simulators." << endl << endl;

	// Build the coordinator
	if (InstallDirs::getDataDir().empty()) {
		throw runtime_error(string(__func__) + "> Data directory not present! Aborting.");
	}

	string traveller_file = "";
	traveller_file = pt_config.get<string>("coordinator.traveller_file");
	traveller_file = (InstallDirs::getDataDir() /= traveller_file).string();

	Coordinator coord(simulator_adapters_raw, traveller_file);

	// Run the simulation.
	const unsigned int num_days = pt_config.get<unsigned int>("coordinator.num_days");
	for (unsigned int i = 0; i < num_days; i++) {
		cout << "Simulating day: " << setw(5) << i;
		coord.timeStep();
		cout << "     Done, infected counts: ";

		cout << setw(10) << simulator_adapters_raw.at(0)->getSimulator().getPopulation()->getInfectedCount();
		for (uint i = 1; i < simulator_adapters_raw.size(); ++i) {
			cout << setw(10) << simulator_adapters_raw.at(i)->getSimulator().getPopulation()->getInfectedCount();
		}
		cout << endl;
	}

	// TODO Generate output files
	// TODO Summary
	// TODO Persons

	// print final message to command line.
	cout << endl << endl;
	cout << "  total time: " << total_clock.toString() << endl << endl;
}

}
