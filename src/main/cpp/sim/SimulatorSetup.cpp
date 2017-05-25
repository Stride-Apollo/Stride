
#include "sim/SimulatorSetup.h"
#include "sim/SimulatorBuilder.h"

#ifdef HDF5_USED
	#include "checkpointing/Loader.h"
#endif

#include "sim/Simulator.h"
#include <boost/filesystem.hpp>


using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;

namespace stride {

SimulatorSetup::SimulatorSetup(string conf_file, string hdf5_file, RunMode run_mode,
							   int num_threads, bool track_index_case, const unsigned int timestamp_replay)
	: m_conf_file(conf_file), m_hdf5_file(hdf5_file), m_num_threads(num_threads),
	  m_timestamp_replay(timestamp_replay), m_track_index_case(track_index_case), m_run_mode(run_mode) {

	m_conf_file_exists = fileExists(m_conf_file);
	m_hdf5_file_exists = fileExists(m_hdf5_file);

	if (run_mode == RunMode::Initial) {
		this->constructConfigTreeInitial();
	} else if (run_mode == RunMode::Extend || run_mode == RunMode::Replay) {
		this->constructConfigTreeExtend();
	}
}


shared_ptr<Simulator> SimulatorSetup::getSimulator() {
	if (m_run_mode == RunMode::Initial) {
		// Build the simulator either from the provided configuration file or the initial data in the hdf5 file.
		if (m_conf_file_exists) {
			return SimulatorBuilder::build(m_pt_config, m_num_threads, m_track_index_case);
		}
		#ifdef HDF5_USED
		else {
			// Check for file validity
			const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
			if (!is_regular_file(file_path_hdf5)) {
				throw runtime_error("Hdf5 file '" +
					system_complete(m_hdf5_file).string() +
					"' is not a regular file. Aborting");
			}

			// Construct the simulator according to the saved configuration data in the hdf5 file.
			Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
			m_timestamp_replay = 0;
			return SimulatorBuilder::build(m_pt_config, loader.getDisease(), loader.getContact(), m_num_threads, m_track_index_case);
		}
		#endif
	}
	#ifdef HDF5_USED
	else if (m_run_mode == RunMode::Extend || m_run_mode == RunMode::Replay) {
		// Build the simulator and adjust it to the most recent/specified saved checkpoint in the hdf5 file.
		const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
		if (!is_regular_file(file_path_hdf5)) {
			throw runtime_error("Hdf5 file '" +
				system_complete(m_hdf5_file).string() +
				"' is not a regular file. Aborting");
		}

		Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
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
	#endif

	return nullptr;
}

void SimulatorSetup::constructConfigTreeInitial() {
	#ifdef HDF5_USED
	if (!m_conf_file_exists && !m_hdf5_file_exists) {
		string error_msg = "Config file '" + system_complete(m_conf_file).string() + "' ";
		error_msg += "and Hdf5 file '" + system_complete(m_hdf5_file).string() + "' both ";
		error_msg += "not present. Aborting.";
		throw runtime_error(error_msg);
	}
	#else
	if (!m_conf_file_exists) {
		string error_msg = "Config file '" + system_complete(m_conf_file).string() + "' not present. Aborting";
		throw runtime_error(error_msg);
	}
	#endif

	if (m_conf_file_exists) {
		// Construct the config tree directly from the configuration file.
		const auto file_path_config = canonical(system_complete(m_conf_file));
		if (!is_regular_file(file_path_config)) {
			throw runtime_error("Configuration file '" +
				system_complete(m_conf_file).string() +
				"' is not a regular file. Aborting");
		}
		read_xml(file_path_config.string(), m_pt_config);
	}
	#ifdef HDF5_USED
	else {
		// Construct the config tree from the initial state saved in the Hdf5 file.
		const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
		if (!is_regular_file(file_path_hdf5)) {
			throw runtime_error("Hdf5 file '" +
				system_complete(m_hdf5_file).string() +
				"' is not a regular file. Aborting");
		}
		Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
		m_pt_config = loader.getConfig();
		m_track_index_case = loader.getTrackIndexCase();
	}
	#endif

	// Additional run configurations.
	if (m_pt_config.get_optional<bool>("run.num_participants_survey") == false) {
		m_pt_config.put("run.num_participants_survey", 1);
	}
}

void SimulatorSetup::constructConfigTreeExtend() {
	#ifdef HDF5_USED
	// Get the config tree from the Hdf5 file.
	if (!m_hdf5_file_exists) {
		throw runtime_error("Trying to run simulator in 'Extend' mode, without Hdf5 file '" +
			system_complete(m_hdf5_file).string() + "' present. Aborting.");
	}

	const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
	if (!is_regular_file(file_path_hdf5)) {
		throw runtime_error("Hdf5 file '" + system_complete(m_hdf5_file).string() + "' is not a regular file. Aborting.");
	}

	Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
	m_pt_config = loader.getConfig();
	m_track_index_case = loader.getTrackIndexCase();
	#endif
}


bool SimulatorSetup::fileExists(string filename) const {
	return exists(system_complete(filename));
}



}