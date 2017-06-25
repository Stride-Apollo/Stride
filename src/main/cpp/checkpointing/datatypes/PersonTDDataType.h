#pragma once

#include "H5Cpp.h"

using namespace H5;

/**
 * Time Dependent Person DataType
 */

namespace stride {
struct PersonTDDataType {
	static CompType getCompType() {
		CompType type_person_TD(sizeof(PersonTDDataType));
#define insertMemberTD(name, attribute, type) type_person_TD.insertMember(H5std_string(name), HOFFSET(PersonTDDataType, attribute), type)
		insertMemberTD("participant", m_participant, PredType::NATIVE_HBOOL);
		insertMemberTD("health_status", m_health_status, PredType::NATIVE_UINT);
		insertMemberTD("disease_counter", m_disease_counter, PredType::NATIVE_UINT);
		insertMemberTD("on_vacation", m_on_vacation, PredType::NATIVE_HBOOL);
#undef insertMemberTD

		return type_person_TD;
	}

	int m_participant;
	unsigned int m_health_status;
	unsigned int m_disease_counter;
	int m_on_vacation;
};
}