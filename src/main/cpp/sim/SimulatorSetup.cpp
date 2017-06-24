
#include "sim/SimulatorSetup.h"
#include "sim/SimulatorBuilder.h"
#include "checkpointing/Hdf5Loader.h"
#include "sim/Simulator.h"
#include <boost/filesystem.hpp>


using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;

namespace stride {

SimulatorSetup::SimulatorSetup(const ptree& pt_config, string hdf5_file, RunMode run_mode,
							   int num_threads, bool track_index_case, const unsigned int timestamp_replay)
	: m_pt_config(pt_config), m_hdf5_file(hdf5_file), m_num_threads(num_threads),
	  m_timestamp_replay(timestamp_replay), m_track_index_case(track_index_case), m_run_mode(run_mode) {

	// TODO m_conf_file_exists not initialized because this no longer requires a file (replaced by ptree)

	m_hdf5_file_exists = fileExists(m_hdf5_file);
}


shared_ptr<Simulator> SimulatorSetup::getSimulator() {
	if (m_run_mode == RunMode::Initial) {
		// Build the simulator either from the provided configuration file or the initial data in the hdf5 file.
		return SimulatorBuilder::build(m_pt_config, m_num_threads, m_track_index_case);
	} else if (m_run_mode == RunMode::Extend || m_run_mode == RunMode::Replay) {
		// Build the simulator and adjust it to the most recent/specified saved checkpoint in the hdf5 file.
		const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
		if (!is_regular_file(file_path_hdf5)) {
			throw runtime_error("Hdf5 file '" +
				system_complete(m_hdf5_file).string() +
				"' is not a regular file. Aborting");
		}

		Hdf5Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
		m_pt_config = loader.getConfig();
		auto sim = SimulatorBuilder::build(loader.getConfig(), loader.getDisease(), loader.getContact(), m_num_threads, m_track_index_case);
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

bool SimulatorSetup::fileExists(string filename) const {
	return exists(system_complete(filename));
}



}