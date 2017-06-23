#pragma once

#include "H5Cpp.h"
using namespace H5;

/**
 * Traveller data (multi region extension)
 */
struct TravellerDataType {
	static CompType getCompType() {
		CompType type_traveller_data(sizeof(TravellerDataType));
		#define insertMemberTraveller(name, attribute, type) type_traveller_data.insertMember(H5std_string(name), HOFFSET(TravellerDataType, attribute), type)
		insertMemberTraveller("home_sim_id", m_home_sim_id, PredType::NATIVE_UINT);
		insertMemberTraveller("dest_sim_id", m_dest_sim_id, PredType::NATIVE_UINT);
		insertMemberTraveller("home_sim_index", m_home_sim_index, PredType::NATIVE_UINT);
		insertMemberTraveller("dest_sim_index", m_dest_sim_index, PredType::NATIVE_UINT);
		insertMemberTraveller("days_left", m_days_left, PredType::NATIVE_UINT);

		insertMemberTraveller("ID", m_ID, PredType::NATIVE_UINT);
		insertMemberTraveller("age", m_age, PredType::NATIVE_DOUBLE);
		insertMemberTraveller("gender", m_gender, PredType::NATIVE_CHAR);
		insertMemberTraveller("household_ID", m_household_ID, PredType::NATIVE_UINT);
		insertMemberTraveller("school_ID", m_school_ID, PredType::NATIVE_UINT);
		insertMemberTraveller("work_ID", m_work_ID, PredType::NATIVE_UINT);
		insertMemberTraveller("prim_comm_ID", m_prim_comm_ID, PredType::NATIVE_UINT);
		insertMemberTraveller("sec_comm_ID", m_sec_comm_ID, PredType::NATIVE_UINT);
		insertMemberTraveller("start_infectiousness", m_start_infectiousness, PredType::NATIVE_UINT);
		insertMemberTraveller("time_infectiousness", m_time_infectiousness, PredType::NATIVE_UINT);
		insertMemberTraveller("start_symptomatic", m_start_symptomatic, PredType::NATIVE_UINT);
		insertMemberTraveller("time_symptomatic", m_time_symptomatic, PredType::NATIVE_UINT);

		insertMemberTraveller("participant", m_participant, PredType::NATIVE_INT);
		insertMemberTraveller("health_status", m_health_status, PredType::NATIVE_UINT);
		insertMemberTraveller("disease_counter", m_disease_counter, PredType::NATIVE_UINT);
		#undef insertMemberTraveller

		return type_traveller_data;
	}

	unsigned int m_home_sim_id;
	unsigned int m_dest_sim_id;
	unsigned int m_home_sim_index;
	unsigned int m_dest_sim_index;

	unsigned int m_days_left;

	unsigned int m_ID;
	double 		 m_age;
	char 		 m_gender;
	unsigned int m_household_ID;
	unsigned int m_school_ID;
	unsigned int m_work_ID;
	unsigned int m_prim_comm_ID;
	unsigned int m_sec_comm_ID;
	unsigned int m_start_infectiousness;
	unsigned int m_time_infectiousness;
	unsigned int m_start_symptomatic;
	unsigned int m_time_symptomatic;

	int 		 m_participant;
	unsigned int m_health_status;
	unsigned int m_disease_counter;

};
