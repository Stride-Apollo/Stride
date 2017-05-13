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
 * Main program: command line handling.
 */

#include "run_stride.h"

#include <tclap/CmdLine.h>

using namespace std;
using namespace stride;
using namespace TCLAP;

/// Main program of the stride simulator.
int main(int argc, char** argv) {
	int exit_status = EXIT_SUCCESS;
	try {
		// Parse command line.
		CmdLine cmd("stride", ' ', "1.0", false);
		SwitchArg index_case_Arg("r", "r0", "R0 only", cmd, false);
		ValueArg <string> config_file_Arg("c", "config", "Config File", false,
				// "./config/run_coordinator.xml", "CONFIGURATION FILE", cmd);
										  "./config/run_default.xml", "CONFIGURATION FILE", cmd);
		ValueArg<unsigned int> num_threads_Arg("n", "num_threads", "Number of threads (as a hint)", false,
											   4, "NUM THREADS", cmd);
		ValueArg<int> checkpointing_frequency_Arg("f", "frequency", "Checkpointing Frequency", false,
												  -1, "CHECKPOINTING FREQUENCY", cmd);
		ValueArg<unsigned int> timestamp_replay_Arg("t", "timestamp", "Replay from Timestamp", false, 0,
													"REPLAY FROM TIMESTAMP", cmd);
		ValueArg<string> simulator_mode_Arg("m", "mode", "Simulator Mode", false,
											 "extend", "SIMULATOR MODE", cmd);
		ValueArg<string> hdf5_file_Arg("h", "hdf5", "HDF5 File", false,
										"", "HDF5 FILE", cmd);
		ValueArg<string> hdf5_output_file_Arg("o", "hdf5output", "HDF5 Output File", false,
											  "", "HDF5 OUTPUT FILE", cmd);
		cmd.parse(argc, argv);

		run_stride(index_case_Arg.getValue(),
				   num_threads_Arg.getValue(),
				   config_file_Arg.getValue(),
				   hdf5_file_Arg.getValue(),
				   hdf5_output_file_Arg.getValue(),
				   simulator_mode_Arg.getValue(),
				   checkpointing_frequency_Arg.getValue(),
				   timestamp_replay_Arg.getValue());

	} catch (exception& e) {
		exit_status = EXIT_FAILURE;
		cerr << "\nEXCEPTION THROWN: " << e.what() << endl;
	} catch (...) {
		exit_status = EXIT_FAILURE;
		cerr << "\nEXCEPTION THROWN: " << "Unknown exception." << endl;
	}

	return exit_status;
}
