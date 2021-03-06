#pragma once
/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * DaysOffStandard class.
 */

#include "Calendar.h"
#include "DaysOffInterface.h"

#include <memory>

namespace stride {

/**
 * Standard situation for days off from work and school.
 */
class DaysOffStandard : public DaysOffInterface {
public:
	/// Initialize calendar.
	DaysOffStandard(std::shared_ptr<Calendar> cal) : m_calendar(cal) {}

	/// See DaysOffInterface.
	bool isWorkOff() override {
		return m_calendar->isWeekend() || m_calendar->isHoliday();
	}

	/// See DaysOffInterface.
	virtual bool isSchoolOff() override {
		return m_calendar->isWeekend() || m_calendar->isHoliday() || m_calendar->isSchoolHoliday();
	}

private:
	std::shared_ptr<Calendar> m_calendar;             ///< Management of calendar.
};

}

