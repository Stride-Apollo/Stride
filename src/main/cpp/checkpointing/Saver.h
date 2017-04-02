#pragma once

/**
 * @file
 * Header file for the Saver class for the checkpointing functionality
 */

#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"
#include <boost/property_tree/xml_parser.hpp>

namespace stride {
using namespace boost::property_tree;

/**
 * Saver class to save to hdf5
 */

// TODO Maybe make this singleton?
class Saver : public util::Observer<Simulator> {
public:
	Saver(const char* filename, ptree pt_config, int frequency);

	virtual void update(const Simulator& sim);


	// TODO After implementing Saver completely, remove this function
	void testLoad();

private:
	const char* m_filename;
	int m_frequency;
	ptree m_pt_config;
	H5::CompType typeConf;

	int m_current_step;
	unsigned int m_timestep;
};

}

