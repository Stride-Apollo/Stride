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
 * Header file for the core Population class
 */

#include "Person.h"
#include "core/Health.h"
#include "sim/Simulator.h"

#include <numeric>
#include <vector>

namespace stride {

/**
 * Container for persons in population.
 */
class Population : public std::vector<Simulator::PersonType> {
public:
	/// Get the cumulative number of cases.
	unsigned int getInfectedCount() const {
		unsigned int total {0U};
		for (const auto& p : *this) {
			const auto& h = p.getHealth();
			total += h.isInfected() || h.isRecovered();
		}
		return total;
	}

	double getFractionInfected() const {
		return getInfectedCount() / this->size();
	}
};

}

