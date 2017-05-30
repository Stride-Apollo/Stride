/*
 * Vaccination.h
 *
 *  Created on: May 29, 2017
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_BEHAVIOUR_BEHAVIOUR_POLICIES_VACCINATION_H_
#define SRC_MAIN_CPP_BEHAVIOUR_BEHAVIOUR_POLICIES_VACCINATION_H_


template<typename belief_policy>
class Vaccination {
public:
	static bool PracticesSocialDistancing(const typename belief_policy::Data& belief_data) {
		return false;
	}

	static bool PracticesVaccination(const typename belief_policy::Data& belief_data) {
		if (belief_policy::HasAdopted(belief_data)) {
			return true;
		} else {
			return false;
		}
	}
};


#endif /* SRC_MAIN_CPP_BEHAVIOUR_BEHAVIOUR_POLICIES_VACCINATION_H_ */
