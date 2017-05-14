#pragma once

#include "H5Cpp.h"
using namespace H5;

/**
 * Traveller data (multi region extension)
 */
struct PersonTDDataType {

	unsigned int m_home_id;
	unsigned int m_home_age;

	unsigned int m_destination_work_id;
	unsigned int m_destination_primary_id;
	unsigned int m_destination_secondary_id;
	unsigned int m_days_left;

	unsigned int m_source_simulator;
	unsigned int m_destination_simulator;
};
