#pragma once

/**
 * @file
 * Header file for the Saver class for the checkpointing functionality
 */


#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"
#include "sim/LocalSimulatorAdapter.h"
#include "core/Cluster.h"
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <vector>

using std::vector;
using namespace boost::property_tree;
using std::string;


namespace stride {

class Saver : public util::Observer<LocalSimulatorAdapter> {
public:
	Saver(string filename, ptree pt_config,
		  int frequency, bool track_index_case,
		  string simulator_run_mode = "initial",
		  int start_timestep = 0);

	/// Update function which is called by the subject.
	virtual void update(const LocalSimulatorAdapter& local_sim);

	/// Forces a save to the hdf5 file, with an optional timestep argument which specifies a new timestep save index.
	void forceSave(const LocalSimulatorAdapter& local_sim, int timestep = -1);

private:
	void saveTimestep(const Simulator& sim);
	void saveClusters(H5::Group& group, string dataset_name, const vector<Cluster>& clusters);

private:
	string m_filename;
	int m_frequency;
	ptree m_pt_config;
	H5::CompType typeConf;

	int m_current_step;
	unsigned int m_timestep;
	unsigned int m_save_count;
};

}
