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
#include "util/ConfigInfo.h"
#include "util/InstallDirs.h"
#include "util/Stopwatch.h"
#include "util/TimeStamp.h"
#include "checkpointing/Saver.h"
#include "checkpointing/Loader.h"

#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
#include <spdlog/spdlog.h>

namespace stride {

using namespace output;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;
using namespace std::chrono;

/// Run the stride simulator.
void run_stride(bool track_index_case, 
				const string& config_file_name,
				const string& hdf5_file_name,
				const string& hdf5_output_file_name,
				const string& simulator_run_mode,
				const int checkpointing_frequency,
				const unsigned int timestamp_replay) {
	// -----------------------------------------------------------------------------------------
	// print output to command line.
	// -----------------------------------------------------------------------------------------
	cout << "\n*************************************************************" << endl;
	cout << "Starting up at:      " << TimeStamp().toString() << endl;
	cout << "Executing:           " << InstallDirs::getExecPath().string() << endl;
	cout << "Current directory:   " << InstallDirs::getCurrentDir().string() << endl;
	cout << "Install directory:   " << InstallDirs::getRootDir().string() << endl;
	cout << "Data    directory:   " << InstallDirs::getDataDir().string() << endl;


	// -----------------------------------------------------------------------------------------
	// check execution environment.
	// -----------------------------------------------------------------------------------------
	if (InstallDirs::getCurrentDir().compare(InstallDirs::getRootDir()) != 0) {
		throw runtime_error(string(__func__) + "> Current directory is not install root! Aborting.");
	}
	if (InstallDirs::getDataDir().empty()) {
		throw runtime_error(string(__func__) + "> Data directory not present! Aborting.");
	}

	// -----------------------------------------------------------------------------------------
	// OpenMP.
	// -----------------------------------------------------------------------------------------
	unsigned int num_threads;
	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	cout << endl;
	if (ConfigInfo::haveOpenMP()) {
		cout << "Using OpenMP threads:  " << num_threads << endl;
	} else {
		cout << "Not using OpenMP threads." << endl;
	}
	cout << endl;

	// -----------------------------------------------------------------------------------------
	// Configuration.
	// -----------------------------------------------------------------------------------------
	
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

	cout << "Constructing configuration tree and building the simulator." << endl << endl;
	
	SimulatorSetup setup = SimulatorSetup(simulator_run_mode, config_file_name, hdf5_file_name, num_threads, track_index_case, timestamp_replay);
	ptree pt_config = setup.getConfigTree();
	shared_ptr<Simulator> sim = setup.getSimulator();
	unsigned int start_day = setup.getStartDay();

	cout << "Done building the simulator." << endl;

	// -----------------------------------------------------------------------------------------
	// Set output path prefix.
	// -----------------------------------------------------------------------------------------
	auto output_prefix = pt_config.get<string>("run.output_prefix", "");
	if (output_prefix.length() == 0) {
		output_prefix = TimeStamp().toTag();
	}
	cout << "Project output tag:  " << output_prefix << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Additional run configurations.
	// -----------------------------------------------------------------------------------------
	if (pt_config.get_optional<bool>("run.num_participants_survey") == false) {
		pt_config.put("run.num_participants_survey", 1);
	}

	// -----------------------------------------------------------------------------------------
	// Track index case setting.
	// -----------------------------------------------------------------------------------------
	cout << "Setting for track_index_case:  " << boolalpha << track_index_case << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Create logger
	// Transmissions:     [TRANSMISSION] <infecterID> <infectedID> <clusterID> <day>
	// General contacts:  [CNT] <person1ID> <person1AGE> <person2AGE>  <at_home> <at_work> <at_school> <at_other>
	// -----------------------------------------------------------------------------------------
	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", output_prefix + "_logfile",
												  std::numeric_limits<size_t>::max(),
												  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging


	// -----------------------------------------------------------------------------------------
	// Add observers to the simulator.
	// -----------------------------------------------------------------------------------------

	cout << "Adding observers to the simulator." << endl;

	// Is checkpointing 'enabled'?
	if (hdf5_file_name != "" || hdf5_output_file_name != "") {
		int frequency = checkpointing_frequency == -1 ?
							pt_config.get<int>("run.checkpointing_frequency") : checkpointing_frequency;
		string output_file = (hdf5_output_file_name == "") ? hdf5_file_name : hdf5_output_file_name;
		auto classInstance = std::make_shared<Saver>
			(Saver(output_file.c_str(), pt_config, frequency, track_index_case, simulator_run_mode, (start_day == 0) ? 0 : start_day + 1));
		std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
		sim->registerObserver(classInstance, fnCaller);
	}
	cout << "Done adding the observers." << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Run the simulation.
	// -----------------------------------------------------------------------------------------
	Stopwatch<> run_clock("run_clock");

	// The initial save
	if (start_day == 0)
		sim->notify(*sim);

	unsigned int num_days;
	if (simulator_run_mode == "extend") {
		// Extend the amount of days that should be run according to the config param or cmd argument
		num_days = start_day + (timestamp_replay == 0 ? pt_config.get<unsigned int>("run.num_days") : timestamp_replay); 
	} else {
		num_days = pt_config.get<unsigned int>("run.num_days");
	}
	vector<unsigned int> cases(num_days);
	
	for (unsigned int i = start_day; i < num_days; i++) {
		cout << "Simulating day: " << setw(5) << i;
		run_clock.start();
		sim->timeStep();
		run_clock.stop();
		cout << "     Done, infected count: ";
		cases[i] = sim->getPopulation()->getInfectedCount();
		cout << setw(10) << cases[i] << endl;
	}

	// -----------------------------------------------------------------------------------------
	// Generate output files
	// -----------------------------------------------------------------------------------------
	// Cases
	CasesFile cases_file(output_prefix);
	cases_file.print(cases);

	// Summary
	SummaryFile summary_file(output_prefix);
	summary_file.print(pt_config,
					   sim->getPopulation()->size(), sim->getPopulation()->getInfectedCount(),
					   duration_cast<milliseconds>(run_clock.get()).count(),
					   duration_cast<milliseconds>(total_clock.get()).count());

	// Persons
	if (pt_config.get<double>("run.generate_person_file") == 1) {
		PersonFile person_file(output_prefix);
		person_file.print(sim->getPopulation());
	}

	// -----------------------------------------------------------------------------------------
	// print final message to command line.
	// -----------------------------------------------------------------------------------------
	cout << endl << endl;
	cout << "  run_time: " << run_clock.toString()
		 << "  -- total time: " << total_clock.toString() << endl << endl;
	cout << "Exiting at:         " << TimeStamp().toString() << endl << endl;
}

}
