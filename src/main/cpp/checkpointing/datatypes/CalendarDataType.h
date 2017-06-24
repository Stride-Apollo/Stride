#pragma once

#include "H5Cpp.h"
using namespace H5;


struct CalendarDataType {
	static CompType getCompType() {
		StrType str_type(0, H5T_VARIABLE);
		CompType type_calendar(sizeof(CalendarDataType));
		type_calendar.insertMember(H5std_string("day"), HOFFSET(CalendarDataType, m_day), PredType::NATIVE_HSIZE);
		type_calendar.insertMember(H5std_string("date"), HOFFSET(CalendarDataType, m_date), str_type);

		return type_calendar;
	}

	std::size_t m_day;
	const char* m_date;
};
