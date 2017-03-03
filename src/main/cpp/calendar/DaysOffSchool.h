#ifndef SRC_MAIN_CALENDAR_DAYS_OFF_SCHOOL_H_
#define SRC_MAIN_CALENDAR_DAYS_OFF_ALL_H_
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
 * DaysOffAll class: everybody gets the day off.
 */

#include "DaysOffInterface.h"

namespace stride {

/**
 * Schools closed.
 */
class DaysOffSchool : public DaysOffInterface
{
public:
        /// Initialize calendar.
        DaysOffSchool(std::shared_ptr<Calendar> cal) : m_calendar(cal) {}

        /// See DaysOffInterface.
        bool isWorkOff() override
        {
                return m_calendar->isWeekend() || m_calendar->isHoliday();
        }

        /// See DaysOffInterface.
        virtual bool isSchoolOff() override { return true; }

private:
        std::shared_ptr<Calendar>           m_calendar;             ///< Management of calendar.
};

} // end_of_namespace

#endif // end of include guard
