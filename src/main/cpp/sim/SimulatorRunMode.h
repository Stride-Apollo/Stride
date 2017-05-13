#pragma once

#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;


namespace stride {

enum RunMode {
	Initial = 0U, Extend = 1U, Replay = 2U, Extract = 3U
};


class SimulatorRunMode {
public:
	static RunMode getRunMode(string run_mode);
	static vector<string> getAcceptedModes();

private:
	static map<string, RunMode> g_name_run_mode;
};

}
