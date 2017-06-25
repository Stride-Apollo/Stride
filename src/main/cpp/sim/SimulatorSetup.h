
/**
 * @file
 * Setup for the simulator and configuration tree.
 */


#pragma once

#include <string>
#include <memory>
#include <boost/property_tree/xml_parser.hpp>
#include "sim/Simulator.h"
#include "sim/SimulatorRunMode.h"

using namespace boost::property_tree;
using namespace std;

namespace stride {

// TODO: At this point SimulatorSetup is so small it could be made into one function
class SimulatorSetup {
public:
	SimulatorSetup(const ptree& config, string hdf5_file, RunMode run_mode,
				   const unsigned int timestamp_replay);

	shared_ptr<Simulator> getSimulator();

	unsigned int getStartDay() const {
		return m_timestamp_replay;
	}

private:
	/// Helper function to check if the file with filename actually exists.
	bool fileExists(string filename) const;

private:
	ptree m_pt_config;
	string m_hdf5_file;
	mutable unsigned int m_timestamp_replay;
	bool m_hdf5_file_exists;
	RunMode m_run_mode;
};

}
