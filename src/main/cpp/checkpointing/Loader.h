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
	Loader(const char* filename, Simulator& sim);

private:
	const char* m_filename;
};

}