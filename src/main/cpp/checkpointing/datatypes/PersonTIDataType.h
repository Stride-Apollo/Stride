#pragma once

#include "H5Cpp.h"
using namespace H5;


/**
 * Time Independent Person DataType
 */
namespace {
struct PersonTIDataType {
	static CompType getCompType() {
		CompType type_person_TI(sizeof(PersonTIDataType));
#define insertMemberTI(name, attribute, type) type_person_TI.insertMember(H5std_string(name), HOFFSET(PersonTIDataType, attribute), type)
		insertMemberTI("id", m_id, PredType::NATIVE_UINT);
		insertMemberTI("age", m_age, PredType::NATIVE_DOUBLE);
		insertMemberTI("gender", m_gender, PredType::NATIVE_CHAR);
		insertMemberTI("household_id", m_household_id, PredType::NATIVE_UINT);
		insertMemberTI("school_id", m_school_id, PredType::NATIVE_UINT);
		insertMemberTI("work_id", m_work_id, PredType::NATIVE_UINT);
		insertMemberTI("prim_comm_id", m_prim_comm_id, PredType::NATIVE_UINT);
		insertMemberTI("sec_comm_id", m_sec_comm_id, PredType::NATIVE_UINT);
		insertMemberTI("start_infectiousness", m_start_infectiousness, PredType::NATIVE_UINT);
		insertMemberTI("time_infectiousness", m_time_infectiousness, PredType::NATIVE_UINT);
		insertMemberTI("start_symptomatic", m_start_symptomatic, PredType::NATIVE_UINT);
		insertMemberTI("time_symptomatic", m_time_symptomatic, PredType::NATIVE_UINT);
#undef insertMemberTI

		return type_person_TI;
	}


	unsigned int m_id;
	double m_age;
	char m_gender;
	unsigned int m_household_id;
	unsigned int m_school_id;
	unsigned int m_work_id;
	unsigned int m_prim_comm_id;
	unsigned int m_sec_comm_id;
	unsigned int m_start_infectiousness;
	unsigned int m_time_infectiousness;
	unsigned int m_start_symptomatic;
	unsigned int m_time_symptomatic;
};
}