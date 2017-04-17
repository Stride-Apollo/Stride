
#include "sim/SimulatorSetup.h"
#include "sim/SimulatorBuilder.h"
#include "checkpointing/Loader.h"
#include "sim/Simulator.h"
#include <boost/filesystem.hpp>


using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;

namespace stride {

SimulatorSetup::SimulatorSetup(string simulator_mode, string conf_file, 
							   string hdf5_file, int num_threads, bool track_index_case) 
	: m_simulator_mode(simulator_mode), m_conf_file(conf_file), m_hdf5_file(hdf5_file), 
	  m_num_threads(num_threads), m_track_index_case(track_index_case) {

	m_conf_file_exists = fileExists(m_conf_file);
	m_hdf5_file_exists = fileExists(m_hdf5_file);

	if (m_simulator_mode == "initial") {
		this->constructConfigTreeInitial();
	} else if (m_simulator_mode == "extend") {
		this->constructConfigTreeExtend();
	} else {
		throw runtime_error(string(__func__) + "> " 
			+ m_simulator_mode + " is not a valid/supported simulator mode.");
	}
}


shared_ptr<Simulator> SimulatorSetup::getSimulator() const {

	if (m_simulator_mode == "initial") {
		// Build the simulator either from the provided configuration file or the initial data in the hdf5 file.
		if (m_conf_file_exists) {
			return SimulatorBuilder::build(m_pt_config, m_num_threads, m_track_index_case);
		} else {
			const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
			if (!is_regular_file(file_path_hdf5)) {
				throw runtime_error(string(__func__) + "> Hdf5 file is not a regular file.");
			}

			Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
			return SimulatorBuilder::build(m_pt_config, loader.get_disease(), loader.get_contact(), m_num_threads, m_track_index_case);
		}
	} else if (m_simulator_mode == "extend") {
		// Build the simulator and adjust it to the most recent saved checkpoint in the hdf5 file.
		const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
		if (!is_regular_file(file_path_hdf5)) {
			throw runtime_error(string(__func__) + "> Hdf5 file is not a regular file.");
		}

		Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
		auto sim = SimulatorBuilder::build(m_pt_config, loader.get_disease(), loader.get_contact(), m_num_threads, m_track_index_case);
		loader.extend_simulation(sim);
		return sim;
	} else {
		// Should not be able to get here (runtime error in constructor), but you never know.
		throw runtime_error(string(__func__) + "> " 
			+ m_simulator_mode + " is not a valid/supported simulator mode.");
	}
}


void SimulatorSetup::constructConfigTreeInitial() {
	if (!m_conf_file_exists && !m_hdf5_file_exists) {
		throw runtime_error( string(__func__)
			+ "> Config file " + system_complete(m_conf_file).string()
			+ " and Hdf5 file " + system_complete(m_hdf5_file).string()
			+ " both not present. Aborting.");
	}

	if (m_conf_file_exists) {
		// Construct the config tree directly from the configuration file.
		const auto file_path_config = canonical(system_complete(m_conf_file));
		if (!is_regular_file(file_path_config)) {
			throw runtime_error(string(__func__) + "> Configuration file is not a regular file.");
		}
		read_xml(file_path_config.string(), m_pt_config);
	} else {
		// Construct the config tree from the initial state saved in the Hdf5 file.
		const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
		if (!is_regular_file(file_path_hdf5)) {
			throw runtime_error(string(__func__) + "> Hdf5 file is not a regular file.");
		}
		Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
		m_pt_config = loader.get_config();	
		m_track_index_case = loader.get_track_index_case();
	}
}

void SimulatorSetup::constructConfigTreeExtend() {
	// Get the config tree from the Hdf5 file.
	if (!m_hdf5_file_exists) {
		throw runtime_error(string(__func__) + "> Hdf5 file " + 
							system_complete(m_hdf5_file).string() + " does not exist.");
	}

	const auto file_path_hdf5 = canonical(system_complete(m_hdf5_file));
	if (!is_regular_file(file_path_hdf5)) {
		throw runtime_error(string(__func__) + "> Hdf5 file is not a regular file.");
	}

	Loader loader(file_path_hdf5.string().c_str(), m_num_threads);
	m_pt_config = loader.get_config();
	m_track_index_case = loader.get_track_index_case();
}

bool SimulatorSetup::fileExists(string filename) const {
	return exists(system_complete(filename));
}


}