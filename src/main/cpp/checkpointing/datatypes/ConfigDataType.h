#pragma once

#include "H5Cpp.h"
using namespace H5;

namespace {
struct ConfigDataType {
	static CompType getCompType() {
		StrType str_type(0, H5T_VARIABLE);
		CompType type_conf_data(sizeof(ConfigDataType));
		type_conf_data.insertMember(H5std_string("config_content"), HOFFSET(ConfigDataType, m_config_content),
									str_type);
		type_conf_data.insertMember(H5std_string("disease_content"), HOFFSET(ConfigDataType, m_disease_content),
									str_type);
		type_conf_data.insertMember(H5std_string("age_contact_content"), HOFFSET(ConfigDataType, m_age_contact_content),
									str_type);
		type_conf_data.insertMember(H5std_string("holidays_content"), HOFFSET(ConfigDataType, m_holidays_content),
									str_type);

		return type_conf_data;
	}

	const char* m_config_content;
	const char* m_disease_content;
	const char* m_holidays_content;
	const char* m_age_contact_content;
};
}