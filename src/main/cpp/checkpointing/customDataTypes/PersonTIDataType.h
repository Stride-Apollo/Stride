#pragma once

#include "H5Cpp.h"
using namespace H5;


/**
 * Time Independent Person DataType
 */
struct PersonTIDataType {
	static CompType getCompType() {
		CompType type_person_TI(sizeof(PersonTIDataType));
		#define insertMemberTI(name, attribute, type) type_person_TI.insertMember(H5std_string(name), HOFFSET(PersonTIDataType, attribute), type)
		insertMemberTI("ID", ID, PredType::NATIVE_UINT);
		insertMemberTI("age", age, PredType::NATIVE_DOUBLE);
		insertMemberTI("gender", gender, PredType::NATIVE_CHAR);
		insertMemberTI("household_ID", household_ID, PredType::NATIVE_UINT);
		insertMemberTI("school_ID", school_ID, PredType::NATIVE_UINT);
		insertMemberTI("work_ID", work_ID, PredType::NATIVE_UINT);
		insertMemberTI("prim_comm_ID", prim_comm_ID, PredType::NATIVE_UINT);
		insertMemberTI("sec_comm_ID", sec_comm_ID, PredType::NATIVE_UINT);
		insertMemberTI("start_infectiousness", start_infectiousness, PredType::NATIVE_UINT);
		insertMemberTI("time_infectiousness", time_infectiousness, PredType::NATIVE_UINT);
		insertMemberTI("start_symptomatic", start_symptomatic, PredType::NATIVE_UINT);
		insertMemberTI("time_symptomatic", time_symptomatic, PredType::NATIVE_UINT);
		#undef insertMemberTI

		return type_person_TI;
	}


	unsigned int ID;
	double age;
	char gender;
	unsigned int household_ID;
	unsigned int school_ID;
	unsigned int work_ID;
	unsigned int prim_comm_ID;
	unsigned int sec_comm_ID;
	unsigned int start_infectiousness;
	unsigned int time_infectiousness;
	unsigned int start_symptomatic;
	unsigned int time_symptomatic;
};
