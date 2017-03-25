#pragma once

/** Time Dependent Person DataType
 */
struct PersonTDDataType {
	int at_household;	// TODO This should be booleans?
	int at_school;
	int at_work;
	int at_prim_comm;
	int at_sec_comm;
	int participant;

	// TODO Compound datatype Health
};
