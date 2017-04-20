
/**
 * @file
 * Setup for the simulator and configuration tree.
 */


#pragma once

#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <memory>
#include "sim/Simulator.h"

using namespace boost::property_tree;
using namespace std;

namespace stride {

class SimulatorSetup {
public:
	SimulatorSetup(string simulator_mode, string conf_file, string hdf5_file, int num_threads, bool track_index_case, const int timestamp_replay);
	
	ptree getConfigTree() const { 
		return m_pt_config; 
	}
	shared_ptr<Simulator> getSimulator() const;

private:
	void constructConfigTreeInitial();
	void constructConfigTreeExtend();

	/// Helper function to check if the file with filename actually exists.
	bool fileExists(string filename) const;

	string		m_simulator_mode;
	string		m_conf_file;
	string		m_hdf5_file;
	int			m_num_threads;
	int			m_timestamp_replay;
	bool		m_track_index_case;
	bool		m_conf_file_exists;
	bool 		m_hdf5_file_exists;
	ptree 		m_pt_config;
};

}