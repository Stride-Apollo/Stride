
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
	for (auto map_entry : SimulatorRunMode::g_name_run_mode) {
		modes.push_back(map_entry.first);
	}
	return modes;
}


RunMode SimulatorRunMode::getRunMode(string run_mode) {
	to_lower(run_mode);
	if (SimulatorRunMode::g_name_run_mode.count(run_mode) != 1) {
		string error_string = "\033[0;31mError: \033[0;35m'" +
			run_mode + "'" + "\033[0m" + " is not a valid run mode." +
		 	"\nAccepted run modes:\n";
		for (auto mode : SimulatorRunMode::getAcceptedModes())
			error_string += "\t* " + mode + '\n';
		throw std::runtime_error(error_string);
	}
	return SimulatorRunMode::g_name_run_mode[run_mode];
}

