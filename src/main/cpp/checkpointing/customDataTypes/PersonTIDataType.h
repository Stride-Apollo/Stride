#pragma once

/** Time Independent Person DataType
 */
struct PersonTIDataType {
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
