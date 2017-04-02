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
	Loader(const char* filename): m_filename(filename) {};
	ptree get_config(unsigned int num_threads);

	bool get_track_index_case() {
		return m_track_index_case;
	};

	ptree get_parse_tree() {
		return m_pt_config;
	}

private:
	const char* m_filename;
	bool m_track_index_case;
	ptree m_pt_config;
};

}