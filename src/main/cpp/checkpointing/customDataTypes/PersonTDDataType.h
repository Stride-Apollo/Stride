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
		insertMemberTD("participant", participant, PredType::NATIVE_HBOOL);
		insertMemberTD("health_status", health_status, PredType::NATIVE_UINT);
		insertMemberTD("disease_counter", disease_counter, PredType::NATIVE_UINT);
		#undef insertMemberTD

		return type_person_TD;
	}

	int participant;
	unsigned int health_status;
	unsigned int disease_counter;
};
