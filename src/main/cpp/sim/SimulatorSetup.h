
/**
 * @file
 * Setup for the simulator and configuration tree.
 */


#pragma once

#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <memory>
#include "sim/Simulator.h"
#include "sim/SimulatorRunMode.h"

using namespace boost::property_tree;
using namespace std;

namespace stride {

class SimulatorSetup {
public:
	SimulatorSetup(const ptree& pt_config, string hdf5_file, RunMode run_mode,
				   int num_threads, bool track_index_case, const unsigned int timestamp_replay);

	ptree getConfigTree() const {
		return m_pt_config;
	}
	shared_ptr<Simulator> getSimulator();

	unsigned int getStartDay() const {
		return m_timestamp_replay;
	}

private:
	/// Helper function to check if the file with filename actually exists.
	bool fileExists(string filename) const;

private:
	string					m_hdf5_file;
	int						m_num_threads;
	mutable unsigned int	m_timestamp_replay;
	bool					m_track_index_case;
	bool 					m_hdf5_file_exists;
	ptree 					m_pt_config;
	RunMode					m_run_mode;
};

}
