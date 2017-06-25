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
 * Main program: command line handling and different runmodes
 */

#include <vector>
#include <string>

#include <tclap/CmdLine.h>

#include "Runner.h"
#include "sim/SimulatorRunMode.h"
#include "checkpointing/Hdf5Loader.h"

using namespace std;
using namespace stride;
using namespace run;
using namespace TCLAP;

int main(int argc, char** argv) {
	int exit_status = EXIT_SUCCESS;
	try {
		Runner::setup();

		// Parse command line.
		CmdLine cmd("stride", ' ', "2.0", false);
		ValueArg<string> config_file_Arg("c", "config", "Config File", false,
										  "", "filename", cmd);
		ValueArg<unsigned int> timestamp_replay_Arg("t", "timestamp", "Replay from Timestamp", false,
													0, "integer", cmd);
		ValueArg<string> simulator_mode_Arg("m", "mode", "Simulator Mode (initial/extend/replay/extract)", false,
											 "initial", "mode", cmd);
		ValueArg<string> hdf5_file_Arg("f", "hdf5_file", "HDF5 file (only used for mode 'extract')", false,
		                               "", "filename", cmd);
		MultiArg<string> overrides_Arg("o", "override", "Override the configuration", false,
		                               "string", cmd);
		cmd.parse(argc, argv);

		auto run_mode = SimulatorRunMode::getRunMode(simulator_mode_Arg.getValue());
		if (run_mode != RunMode::Extract and config_file_Arg.getValue() == "") {
			cout << "All modes except 'extract' require a configuration file (-c flag)." << endl;
			return EXIT_FAILURE;
		}

		if (run_mode == RunMode::Extract) {
			if (hdf5_file_Arg.getValue() == "") {
				cout << "'extract' mode requires a file (-f flag)" << endl;
				return EXIT_FAILURE;
			}

			// TODO: where to write this?
			Hdf5Loader::extractConfigs(hdf5_file_Arg.getValue());
		} else {
			Runner runner(overrides_Arg.getValue(), config_file_Arg.getValue(), run_mode,
						  slave_Arg.getValue(), timestamp_replay_Arg.getValue());
			runner.printInfo();
			runner.initSimulators();
			runner.run();
		}
	} catch (exception& e) {
		exit_status = EXIT_FAILURE;
		cerr << endl << "Exception: " << e.what() << endl;
	} catch (...) {
		exit_status = EXIT_FAILURE;
		cerr << endl << "Unknown exception thrown." << endl;
	}

	return exit_status;
}
