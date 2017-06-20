#pragma once

/**
 * @file
 * Header file for the Saver class for the checkpointing functionality
 */


#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"
#include "sim/SimulatorRunMode.h"
// #include "sim/LocalSimulatorAdapter.h"
#include "core/Cluster.h"
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <vector>

using std::vector;
using namespace boost::property_tree;
using std::string;
using H5::H5File;
using H5::Group;


namespace stride {

class Saver : public util::Observer<Simulator> {
public:
	Saver(string filename, const ptree& pt_config,
		  int frequency, bool track_index_case,
		  RunMode run_mode = RunMode::Initial,
		  int start_timestep = 0);

	/// Update function which is called by the subject.
	virtual void update(const Simulator& sim);

	/// Forces a save to the hdf5 file, with an optional timestep argument which specifies a new timestep save index.
	void forceSave(const Simulator& sim, int timestep = -1);

private:
	void saveTimestep(const Simulator& sim);

	/// Save the order of persons for a cluster group.
	void saveClusters(Group& group, string dataset_name, const vector<Cluster>& clusters) const;

	/// Saves the time indepent person data.
	void savePersonTIData(H5File& file, const Simulator& sim) const;

	/// Saves the time dependent person data.
	void savePersonTDData(Group& group, const Simulator& sim) const;

	/// Saves the total amount of timesteps and current timestep (create will create the dataset first, otherwise open).
	void saveTimestepMetadata(H5File& file, unsigned int total_amt, unsigned int current, bool create = false) const;

	/// Saves the state of the rng (NOTE only used with unipar dummy implementation).
	void saveRngState(Group& group, const Simulator& sim) const;

	/// Saves the calendar.
	void saveCalendar(Group& group, const Simulator& sim) const;

	/// Save all the configuration files (indirectly via the 'main' config file).
	void saveConfigs(H5File& file, const ptree& pt_config) const;

	/// Save the track index case.
	void saveTrackIndexCase(H5File& file, bool track_index_case) const;


private:
	string m_filename;
	int m_frequency;
	int m_current_step;
	unsigned int m_timestep;
	unsigned int m_save_count;
};

}
