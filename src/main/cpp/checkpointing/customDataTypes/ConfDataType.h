#pragma once

#include "H5Cpp.h"
using namespace H5;

struct ConfDataType {
	static CompType getCompType() {
		StrType str_type(0, H5T_VARIABLE);
		CompType type_conf_data(sizeof(ConfDataType));
		type_conf_data.insertMember(H5std_string("conf_content"), HOFFSET(ConfDataType, conf_content), str_type);
		type_conf_data.insertMember(H5std_string("disease_content"), HOFFSET(ConfDataType, disease_content), str_type);
		type_conf_data.insertMember(H5std_string("age_contact_content"), HOFFSET(ConfDataType, age_contact_content), str_type);
		type_conf_data.insertMember(H5std_string("holidays_content"), HOFFSET(ConfDataType, holidays_content), str_type);

		return type_conf_data;
	}

	const char* conf_content;
	const char* disease_content;
	const char* holidays_content;
	const char* age_contact_content;
};
