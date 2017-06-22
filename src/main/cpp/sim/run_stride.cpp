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
#include "sim/SimulatorRunMode.h"
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
#include <stdlib.h>

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
				const int checkpointing_frequency,
				const unsigned int timestamp_replay,
				RunMode run_mode) {

	// Special case for extract mode -> don't run the simulator, just extract the config file.
	if (run_mode == RunMode::Extract) {
		Loader::extractConfigs(hdf5_file_name);
		exit(EXIT_SUCCESS);
	}

	cout << "Loading configuration" << endl;

	SimulatorSetup setup = SimulatorSetup(config_file_name, hdf5_file_name, run_mode, num_threads, track_index_case, timestamp_replay);
	ptree pt_config = setup.getConfigTree();

	cout << "Building the simulator." << endl << endl;

	shared_ptr<Simulator> sim = setup.getSimulator();
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());
	Coordinator coord({local_sim.get()});

	cout << "Done building the simulator. " << endl << endl;
	unsigned int start_day = setup.getStartDay();

	// Set output path prefix.
	string output_prefix = pt_config.get<string>("run.output_prefix", TimeStamp().toTag());
	cout << "Project output tag:  " << output_prefix << endl << endl;

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



	cout << "Adding observers to the simulator." << endl;

	std::shared_ptr<Saver> saver = nullptr;
	std::string config_hdf5_file = pt_config.get<string>("run.checkpointing_file", "");

	// Is checkpointing 'enabled'?
	if (hdf5_file_name != "" || hdf5_output_file_name != "" || config_hdf5_file != "") {
		cout << "Checkpointing enabled." << endl;
		int frequency = checkpointing_frequency == -1 ?
						pt_config.get<int>("run.checkpointing_frequency", 1) : checkpointing_frequency;
		string output_file = (hdf5_output_file_name == "") ? hdf5_file_name : hdf5_output_file_name;
		if (output_file == "") {
			output_file = config_hdf5_file;
		}
		saver = std::make_shared<Saver>
				(Saver(output_file.c_str(), pt_config, frequency, track_index_case, run_mode, (start_day == 0) ? 0 : start_day + 1));
		std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, saver, std::placeholders::_1);
		local_sim->registerObserver(saver, fnCaller);
	}


	if (pt_config.get<bool>("run.visualization", false) == true) {
		auto ClusterSaver_instance = make_shared<ClusterSaver>(output_prefix + "cluster_output", output_prefix + "pop_output");
		auto fn_caller_ClusterSaver = bind(&ClusterSaver::update, ClusterSaver_instance, std::placeholders::_1);
		local_sim->registerObserver(ClusterSaver_instance, fn_caller_ClusterSaver);

		ClusterSaver_instance->update(*local_sim);
	}

	cout << "Done adding the observers." << endl << endl;

	// initial save
	if (saver != nullptr && !(run_mode == RunMode::Extend && start_day != 0)) {
		saver->forceSave(*local_sim);
	}

	// Run the simulation.
	const unsigned int num_days = pt_config.get<unsigned int>("run.num_days");
	vector<unsigned int> cases(num_days);
	Stopwatch<> run_clock("run_clock");

	for (unsigned int i = start_day; i < start_day + num_days; i++) {
		cout << "Simulating day: " << setw(5) << i;
		coord.timeStep();
		cout << "     Done, infected count: ";
		cases.at(i-start_day) = sim->getPopulation()->getInfectedCount();
		unsigned int adopters = sim->getPopulation()->getAdoptedCount<Simulator::BeliefPolicy>();
		cout << setw(7) << cases.at(i-start_day) << "     Adopters count: " << setw(7) << adopters << endl;
	}

	if (saver != nullptr && checkpointing_frequency == 0) {
		// Force save the last timestep in case of frequency 0
		saver->forceSave(*local_sim, num_days + start_day);
	}

	// Generate output files
	// Cases
	CasesFile cases_file(output_prefix);
	cases_file.print(cases);

	// Summary
	SummaryFile summary_file(output_prefix);
	summary_file.print(pt_config,
					   sim->getPopulation()->size(), sim->getPopulation()->getInfectedCount(),
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
