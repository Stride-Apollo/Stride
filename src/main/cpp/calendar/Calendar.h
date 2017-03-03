#ifndef SRC_MAIN_CPP_CALENDAR_CALENDAR_H_
#define SRC_MAIN_CPP_CALENDAR_CALENDAR_H_
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
 * Header file for the Calendar class.
 */

#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>

namespace stride {

/**
 * Class that keeps track of the 'state' of simulated world.
 * E.g. what day it is, holidays, quarantines, ...
 */
class Calendar
{
public:
	/// Constructor
	Calendar(const boost::property_tree::ptree& pt_config);

	/// Advance the internal calendar by one day
	void advanceDay();

	/// Get the current day of the month
	std::size_t getDay() const { return m_date.day(); }

        /// Get the current day of the week (0 (Sunday), ..., 6 (Saturday))
	std::size_t getDayOfTheWeek() const { return m_date.day_of_week(); }

	/// Get the current month
	std::size_t getMonth() const { return m_date.month(); }

        /// Get the current day of the simulation
	std::size_t getSimulationDay() const { return m_day; }

	/// Get the current year
	std::size_t getYear() const { return m_date.year(); }

    /// Check if it's a holiday
    bool isHoliday() const { return (std::find(m_holidays.begin(), m_holidays.end(), m_date) != m_holidays.end()); }

    /// Check if it's a school holiday
    bool isSchoolHoliday() const { return (std::find(m_school_holidays.begin(), m_school_holidays.end(), m_date) != m_school_holidays.end()); }

	/// Check if it's the weekend
	bool isWeekend() const { return (getDayOfTheWeek() == 6 || getDayOfTheWeek() == 0); }

private:
	///
	void initializeHolidays(const boost::property_tree::ptree& pt_config);

private:
	std::size_t                            m_day;                     ///< The current simulation day
	boost::gregorian::date                 m_date;                    ///< The current simulated day
	std::vector<boost::gregorian::date>    m_holidays;                ///< Vector of general holidays
	std::vector<boost::gregorian::date>    m_school_holidays;         ///< Vector of school holidays
};

} // end_of_namespace

#endif // end of include guard
