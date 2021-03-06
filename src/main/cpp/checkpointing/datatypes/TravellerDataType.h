#pragma once

#include "H5Cpp.h"

using namespace H5;

/**
 * Traveller data (multi region extension)
 */
namespace stride {
struct TravellerDataType {
	static CompType getCompType() {
		StrType str_type(0, H5T_VARIABLE);
		CompType type_traveller_data(sizeof(TravellerDataType));

#define insertMemberTraveller(name, attribute, type) type_traveller_data.insertMember(H5std_string(name), HOFFSET(TravellerDataType, attribute), type)
		insertMemberTraveller("home_sim_name", m_home_sim_name, str_type);
		insertMemberTraveller("dest_sim_name", m_dest_sim_name, str_type);
		insertMemberTraveller("home_sim_index", m_home_sim_index, PredType::NATIVE_UINT);
		insertMemberTraveller("dest_sim_index", m_dest_sim_index, PredType::NATIVE_UINT);
		insertMemberTraveller("days_left", m_days_left, PredType::NATIVE_UINT);

		insertMemberTraveller("orig_id", m_orig_id, PredType::NATIVE_UINT);
		insertMemberTraveller("age", m_age, PredType::NATIVE_DOUBLE);
		insertMemberTraveller("gender", m_gender, PredType::NATIVE_CHAR);
		insertMemberTraveller("orig_household_id", m_orig_household_id, PredType::NATIVE_UINT);
		insertMemberTraveller("orig_school_id", m_orig_school_id, PredType::NATIVE_UINT);
		insertMemberTraveller("orig_work_id", m_orig_work_id, PredType::NATIVE_UINT);
		insertMemberTraveller("orig_prim_comm_id", m_orig_prim_comm_id, PredType::NATIVE_UINT);
		insertMemberTraveller("orig_sec_comm_id", m_orig_sec_comm_id, PredType::NATIVE_UINT);
		insertMemberTraveller("start_infectiousness", m_start_infectiousness, PredType::NATIVE_UINT);
		insertMemberTraveller("time_infectiousness", m_time_infectiousness, PredType::NATIVE_UINT);
		insertMemberTraveller("start_symptomatic", m_start_symptomatic, PredType::NATIVE_UINT);
		insertMemberTraveller("time_symptomatic", m_time_symptomatic, PredType::NATIVE_UINT);

		insertMemberTraveller("participant", m_participant, PredType::NATIVE_INT);
		insertMemberTraveller("health_status", m_health_status, PredType::NATIVE_UINT);
		insertMemberTraveller("disease_counter", m_disease_counter, PredType::NATIVE_UINT);
		insertMemberTraveller("new_id", m_new_id, PredType::NATIVE_UINT);
		insertMemberTraveller("new_household_id", m_new_household_id, PredType::NATIVE_UINT);
		insertMemberTraveller("new_school_id", m_new_school_id, PredType::NATIVE_UINT);
		insertMemberTraveller("new_work_id", m_new_work_id, PredType::NATIVE_UINT);
		insertMemberTraveller("new_prim_comm_id", m_new_prim_comm_id, PredType::NATIVE_UINT);
		insertMemberTraveller("new_sec_comm_id", m_new_sec_comm_id, PredType::NATIVE_UINT);
#undef insertMemberTraveller

		return type_traveller_data;
	}

	const char* m_home_sim_name;
	const char* m_dest_sim_name;
	unsigned int m_home_sim_index;
	unsigned int m_dest_sim_index;

	unsigned int m_days_left;

	unsigned int m_orig_id;
	double m_age;
	char m_gender;
	unsigned int m_orig_household_id;
	unsigned int m_orig_school_id;
	unsigned int m_orig_work_id;
	unsigned int m_orig_prim_comm_id;
	unsigned int m_orig_sec_comm_id;
	unsigned int m_start_infectiousness;
	unsigned int m_time_infectiousness;
	unsigned int m_start_symptomatic;
	unsigned int m_time_symptomatic;

	int m_participant;
	unsigned int m_health_status;
	unsigned int m_disease_counter;
	unsigned int m_new_id;
	unsigned int m_new_household_id;
	unsigned int m_new_school_id;
	unsigned int m_new_work_id;
	unsigned int m_new_prim_comm_id;
	unsigned int m_new_sec_comm_id;

};
}