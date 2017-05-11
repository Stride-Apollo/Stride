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

# include "run_stride.h"

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
#include <util/async.h>
#include "util/TimeStamp.h"

#include "vis/ClusterSaver.h"
#include <boost/property_tree/xml_parser.hpp>
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
void run_stride(bool track_index_case, const string& config_file_name) {
	// C++ example of how a Python main loop would look

	// Configuration
	ptree pt_config;
	const auto file_path = canonical(system_complete(config_file_name));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ ">Config file " + file_path.string() + " not present. Aborting.");
	}
	read_xml(file_path.string(), pt_config);
	cout << "Configuration file:  " << file_path.string() << endl ;

	// OpenMP
	unsigned int num_threads = 8; // TODO
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
	cout << "Building the simulator. " << endl;
	auto sim = SimulatorBuilder::build(pt_config, num_threads, track_index_case);
	cout << "Done building the simulator. " << endl << endl;

	// No observers (yet) in C++. Logger was never intended as an observer per timestep.

	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());

	cout << "Adding observers to the simulator." << endl;
	/// example on how to use:
	auto classInstance = std::make_shared<ClusterSaver>("cluster_output");
	std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&ClusterSaver::update, classInstance, std::placeholders::_1);
	local_sim->registerObserver(classInstance, fnCaller);
	cout << "Done adding the observers." << endl << endl;

	// Run the simulation.
	const unsigned int num_days = pt_config.get<unsigned int>("run.num_days");
	vector<unsigned int> cases(num_days);
	Stopwatch<> run_clock("run_clock");

	for (unsigned int i = 0; i < num_days; i++) {
		cout << "Simulating day: " << setw(5) << i;
		run_clock.start();

		vector<future<bool>> fut_results;
		fut_results.push_back(local_sim->timeStep());
		future_pool(fut_results);

		run_clock.stop();
		cout << "     Done, infected count: ";
		cases[i] = sim->getPopulation()->getInfectedCount();
		cout << setw(10) << cases[i] << endl;
	}

	// Generate output files
	// Cases
	CasesFile cases_file(output_prefix);
	cases_file.print(cases);

	// No
	// Summary

	// print final message to command line.
	cout << endl << endl;
	cout << "  total time: " << total_clock.toString() << endl << endl;
}

}
