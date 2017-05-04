#pragma once

/**
 * @file
 * Header file for the Saver class for the checkpointing functionality
 */

#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"
#include "core/Cluster.h"
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <vector>

using std::vector;

namespace stride {
using namespace boost::property_tree;

/**
 * Saver class to save to hdf5
 */

class Saver : public util::Observer<Simulator> {
public:
	Saver(std::string filename, ptree pt_config,
		  int frequency, bool track_index_case,
		  std::string simulator_run_mode = "initial",
		  int start_timestep = 0);

	virtual void update(const Simulator& sim);
	void forceSave(const Simulator& sim, int timestep = -1);

private:
	void saveTimestep(const Simulator& sim);
	void saveClusters(H5::Group& group, std::string dataset_name, const vector<Cluster>& clusters);


	std::string m_filename;
	int m_frequency;
	ptree m_pt_config;
	H5::CompType typeConf;

	int m_current_step;
	unsigned int m_timestep;
	unsigned int m_save_count;
};

}
