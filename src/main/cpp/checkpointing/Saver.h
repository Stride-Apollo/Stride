#pragma once

/**
 * @file
 * Header file for the Saver class for the checkpointing functionality
 */

#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"

namespace stride {

/**
 * Saver class to save to hdf5
 */
class Saver : public util::Observer<Simulator> {
public:
	Saver(const char* filename);

	virtual void update(const Simulator& sim);

private:
	const char* m_filename;
};

}

