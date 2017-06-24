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

#include "behaviour/belief_data/Nothing.h"
#include "core/Health.h"

namespace stride {

template<typename BehaviourPolicy, typename BeliefPolicy>
class Person;

class NoBelief {
public:
	using Data = Nothing;

	static void initialize(Data& belief_data, double risk_averseness) {}

	static void update(Data& belief_data, Health& health_data) {}

	template<typename BehaviourPolicy>
	static void update(Data& belief_data, const Person<BehaviourPolicy, NoBelief>* p) {}

	static bool hasAdopted(const Data& belief_data) { return false; }
};

}
