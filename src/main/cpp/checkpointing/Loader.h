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
	Loader(const char* filename, unsigned int num_threads);

	ptree get_config() {
		return m_pt_config;
	}

	ptree get_disease() {
		return m_pt_disease;
	}

	ptree get_contact() {
		return m_pt_contact;
	}

	void load_from_timestep(unsigned int timestep, std::shared_ptr<Simulator> sim);

	void extend_simulation(std::shared_ptr<Simulator> sim);

	void setup_population(std::shared_ptr<Simulator> sim);

	bool get_track_index_case() {
		return m_track_index_case;
	};

private:
	const char* m_filename;
	bool m_track_index_case;
	unsigned int m_num_threads;
	ptree m_pt_config;
	ptree m_pt_disease;
	ptree m_pt_contact;
};

}