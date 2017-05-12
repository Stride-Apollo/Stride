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

#include "run_stride.h"

#include "output/CasesFile.h"
#include "output/PersonFile.h"
#include "output/SummaryFile.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "sim/SimulatorSetup.h"
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
void run_stride(bool track_index_case,
				unsigned int num_threads,
				const string& config_file_name,
				const string& hdf5_file_name,
				const string& hdf5_output_file_name,
				const string& simulator_run_mode,
				const int checkpointing_frequency,
				const unsigned int timestamp_replay) {

	cout << "Loading configuration" << endl;
	SimulatorSetup setup = SimulatorSetup(simulator_run_mode, config_file_name, hdf5_file_name, num_threads, track_index_case, timestamp_replay);
	ptree pt_config = setup.getConfigTree();

	// Set output path prefix.
	string output_prefix = "";

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

	// Special case for extract mode -> don't run the simulator, just extract the config file.
	if (simulator_run_mode == "extract") {
		bool hdf5_file_exists = exists(system_complete(hdf5_file_name));
		if (!hdf5_file_exists) {
			throw runtime_error(string(__func__) + "> Hdf5 file " +
								system_complete(hdf5_file_name).string() + " does not exist.");
		}

		const auto file_path_hdf5 = canonical(system_complete(hdf5_file_name));
		if (!is_regular_file(file_path_hdf5)) {
			throw runtime_error(string(__func__) + "> Hdf5 file is not a regular file.");
		}

		Loader loader(file_path_hdf5.string().c_str());
		return;
	}

	cout << "Building the simulator." << endl << endl;

	shared_ptr<Simulator> sim = setup.getSimulator();
	unsigned int start_day = setup.getStartDay();
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());

	cout << "Done building the simulator. " << endl << endl;

	Coordinator coord({local_sim.get()});

	cout << "Adding observers to the simulator." << endl;

	std::shared_ptr<Saver> saver = 0;
	// Is checkpointing 'enabled'?
	std::string config_hdf5_file = pt_config.get<string>("run.checkpointing_file", "");

	if (hdf5_file_name != "" || hdf5_output_file_name != "" || config_hdf5_file != "") {
		int frequency = checkpointing_frequency == -1 ?
						pt_config.get<int>("run.checkpointing_frequency") : checkpointing_frequency;
		string output_file = (hdf5_output_file_name == "") ? hdf5_file_name : hdf5_output_file_name;
		if (output_file == "") {
			output_file = config_hdf5_file;
		}
		saver = std::make_shared<Saver>
				(Saver(output_file.c_str(), pt_config, frequency, track_index_case, simulator_run_mode, (start_day == 0) ? 0 : start_day + 1));
		std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, saver, std::placeholders::_1);
		local_sim->registerObserver(saver, fnCaller);
		auto classInstance = std::make_shared<ClusterSaver>("cluster_output");
		std::function<void(const LocalSimulatorAdapter&)> fnCaller2 = std::bind(&ClusterSaver::update, classInstance, std::placeholders::_1);
		local_sim->registerObserver(classInstance, fnCaller2);
	}


	// TODO add option to turn checkpointing on/off
	auto ClusterSaver_instance = make_shared<ClusterSaver>("cluster_output");
	auto fn_caller_ClusterSaver = bind(&ClusterSaver::update, ClusterSaver_instance, std::placeholders::_1);
	local_sim->registerObserver(ClusterSaver_instance, fn_caller_ClusterSaver);
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
		unsigned int adopters = sim->getPopulation()->getAdoptedCount<Simulator::BeliefPolicy>();
		cout << setw(7) << cases[i] << "     Adopters count: " << setw(7) << adopters << endl;
	}

	if (saver != 0 && checkpointing_frequency == 0) {
		// Force save the last timestep
		saver->forceSave(*local_sim, num_days);
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
