#pragma once

#include "H5Cpp.h"
using namespace H5;

/**
 * Time Dependent Person DataType
 */
struct PersonTDDataType {
	static CompType getCompType() {
		CompType type_person_TD(sizeof(PersonTDDataType));
		#define insertMemberTD(name, attribute, type) type_person_TD.insertMember(H5std_string(name), HOFFSET(PersonTDDataType, attribute), type)
		insertMemberTD("at_household", at_household, PredType::NATIVE_HBOOL);
		insertMemberTD("at_school", at_school, PredType::NATIVE_HBOOL);
		insertMemberTD("at_work", at_work, PredType::NATIVE_HBOOL);
		insertMemberTD("at_prim_comm", at_prim_comm, PredType::NATIVE_HBOOL);
		insertMemberTD("at_sec_comm", at_sec_comm, PredType::NATIVE_HBOOL);
		insertMemberTD("participant", participant, PredType::NATIVE_HBOOL);
		insertMemberTD("health_status", health_status, PredType::NATIVE_UINT);
		insertMemberTD("disease_counter", disease_counter, PredType::NATIVE_UINT);
		#undef insertMemberTD
		
		return type_person_TD;
	}

	int at_household;
	int at_school;
	int at_work;
	int at_prim_comm;
	int at_sec_comm;
	int participant;
	unsigned int health_status;
	unsigned int disease_counter;
};
