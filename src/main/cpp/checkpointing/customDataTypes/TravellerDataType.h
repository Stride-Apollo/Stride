#pragma once

#include "H5Cpp.h"
using namespace H5;

/**
 * Traveller data (multi region extension)
 */
struct TravellerType {
	static CompType getCompType() {
		CompType type_traveller_data(sizeof(TravellerType));
		type_traveller_data.insertMember(H5std_string("home_sim_id"), HOFFSET(TravellerType, m_home_sim_id), PredType::NATIVE_UINT);
		type_traveller_data.insertMember(H5std_string("dest_sim_id"), HOFFSET(TravellerType, m_dest_sim_id), PredType::NATIVE_UINT);
		type_traveller_data.insertMember(H5std_string("home_sim_index"), HOFFSET(TravellerType, m_home_sim_index), PredType::NATIVE_UINT);
		type_traveller_data.insertMember(H5std_string("dest_sim_index"), HOFFSET(TravellerType, m_dest_sim_index), PredType::NATIVE_UINT);
		type_traveller_data.insertMember(H5std_string("days_left"), HOFFSET(TravellerType, m_days_left), PredType::NATIVE_UINT);

		return type_traveller_data;
	}

	unsigned int m_home_sim_id;
	unsigned int m_dest_sim_id;
	unsigned int m_home_sim_index;
	unsigned int m_dest_sim_index;

	unsigned int m_days_left;
};
