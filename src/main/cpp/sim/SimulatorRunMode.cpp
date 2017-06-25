
#include "sim/SimulatorRunMode.h"
#include <map>
#include <boost/algorithm/string.hpp>

using namespace stride;
using namespace std;
using boost::to_lower;


map<string, RunMode> SimulatorRunMode::g_name_run_mode {
		make_pair("initial", RunMode::Initial),
		make_pair("extend", RunMode::Extend),
		make_pair("replay", RunMode::Replay),
		make_pair("extract", RunMode::Extract)
};


vector<string> SimulatorRunMode::getAcceptedModes() {
	vector<string> modes;
	#ifdef HDF5_USED
	for (auto map_entry : SimulatorRunMode::g_name_run_mode) {
		modes.push_back(map_entry.first);
	}
	#else
	// Only allow initial mode when hdf5 is disabled
	modes.push_back("initial");
	#endif
	return modes;
}


RunMode SimulatorRunMode::getRunMode(string run_mode) {
	to_lower(run_mode);
	if (SimulatorRunMode::g_name_run_mode.count(run_mode) != 1) {
		string error_string = "\033[0;31mError: \033[0;35m'" +
							  run_mode + "'" + "\033[0m" + " is not a valid run mode." +
							  "\nAccepted run mode(s):\n";
		for (auto mode : SimulatorRunMode::getAcceptedModes())
			error_string += "\t* " + mode + '\n';
		throw std::runtime_error(error_string);
	}

	// Exception when HDF5 is forced off -> only Initial mode is valid.
	#ifndef HDF5_USED
	if (SimulatorRunMode::g_name_run_mode[run_mode] != RunMode::Initial) {
		throw std::runtime_error(string("\033[0;31m") + "Error: " + string("\033[0m") + "Only run mode " +
			 string("\033[0;35m") + "'Initial'" + string("\033[0m") + " is allowed when running without HDF5.\n");
	}
	#endif

	return SimulatorRunMode::g_name_run_mode[run_mode];
}

