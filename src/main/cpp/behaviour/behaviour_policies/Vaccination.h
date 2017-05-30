/*
 * Vaccination.h
 *
 *  Created on: May 29, 2017
 *      Author: elise
 */

#pragma once


template<typename belief_policy>
class Vaccination {
public:
	static bool practicesSocialDistancing(const typename belief_policy::Data& belief_data) {
		return false;
	}

	static bool practicesVaccination(const typename belief_policy::Data& belief_data) {
		return belief_policy::hasAdopted(belief_data);
	}
};

