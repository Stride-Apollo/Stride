#pragma once

/**
 * @file
 * Header file for the Loader class for the checkpointing functionality
 */

#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"
#include <boost/property_tree/xml_parser.hpp>

namespace stride {
using namespace boost::property_tree;

/**
 * Loader class to load simulation from hdf5
 */

class Loader {
public:
	// Special constructor; Only task is to extract config files.
	Loader(const char* filename);

	Loader(const char* filename, unsigned int num_threads);

	ptree getConfig() {
		return m_pt_config;
	}

	ptree getDisease() {
		return m_pt_disease;
	}

	ptree getContact() {
		return m_pt_contact;
	}

	void updateClusterImmuneIndices(std::shared_ptr<Simulator> sim) const;


	void loadFromTimestep(unsigned int timestep, std::shared_ptr<Simulator> sim);

	void extendSimulation(std::shared_ptr<Simulator> sim);

	void loadClusters(H5::H5File& file, std::string dataset_name, std::vector<Cluster>& cluster, std::shared_ptr<Population> pop);

	void setupPopulation(std::shared_ptr<Simulator> sim);

	bool getTrackIndexCase() {
		return m_track_index_case;
	};

	int getLastSavedTimestep() const;

private:
	const char* m_filename;
	bool m_track_index_case;
	unsigned int m_num_threads;
	ptree m_pt_config;
	ptree m_pt_disease;
	ptree m_pt_contact;

};

}