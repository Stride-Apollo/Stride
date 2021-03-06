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
 * DaysOffNone class.
 */

#include "DaysOffInterface.h"

namespace stride {

/**
 * No  days off work or school.
 */
class DaysOffNone : public DaysOffInterface {
public:
	/// Initialize calendar.
	DaysOffNone(std::shared_ptr <Calendar> cal) {}

	/// See DaysOffInterface.
	bool isWorkOff() override { return false; }

	/// See DaysOffInterface.
	bool isSchoolOff() override { return false; }
};

}

