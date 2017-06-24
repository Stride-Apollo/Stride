
#include "sim/SimulatorSetup.h"
#include "sim/SimulatorBuilder.h"

#ifdef HDF5_USED
	#include "checkpointing/Loader.h"
#endif

#include "sim/Simulator.h"
#include <boost/filesystem.hpp>
#include <iostream>

using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;

namespace stride {

SimulatorSetup::SimulatorSetup(const ptree& config, string hdf5_file, RunMode run_mode,
							   const unsigned int timestamp_replay)
	: m_pt_config(config), m_hdf5_file(hdf5_file),
	  m_timestamp_replay(timestamp_replay), m_run_mode(run_mode) {

	m_hdf5_file_exists = fileExists(m_hdf5_file);

	this->constructConfigTree();
}

// TODO: Use HDF5_USED

shared_ptr<Simulator> SimulatorSetup::getSimulator() {
	if (m_run_mode == RunMode::Initial) {
		return SimulatorBuilder::build(m_pt_config);
	} else if (m_run_mode == RunMode::Extend || m_run_mode == RunMode::Replay) {
		// Build the simulator and adjust it to the most recent/specified saved checkpoint in the hdf5 file.

		Loader loader(m_hdf5_file.c_str());

		if (loader.getConfig() != m_pt_config) {
			std::cerr << "WARNING: The configuration in the HDF5 file differs from the one given." << endl;
			std::cerr << "         Use the extract mode to get the configuration saved in the HDF5 file." << endl;
		}

		auto sim = SimulatorBuilder::build(m_pt_config, loader.getDisease(), loader.getContact());
		if (m_run_mode == RunMode::Extend) {
			loader.extendSimulation(sim);
			m_timestamp_replay = loader.getLastSavedTimestep();
		} else {
			loader.loadFromTimestep(m_timestamp_replay, sim);
		}
		return sim;
	}

	return nullptr;
}

void SimulatorSetup::constructConfigTree() {
	if (not m_hdf5_file_exists) {
		throw runtime_error("valid Hdf5 file '" + m_hdf5_file + "' not present.");
	}

	// Additional run configurations.
	if (m_pt_config.get_optional<bool>("run.num_participants_survey") == false) {
		m_pt_config.put("run.num_participants_survey", 1);
	}
}

bool SimulatorSetup::fileExists(string filename) const {
	return exists(filename) and is_regular_file(filename);
}

}