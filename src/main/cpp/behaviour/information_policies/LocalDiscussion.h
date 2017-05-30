/*
 * LocalDiscussion.h
 *
 *  Created on: May 26, 2017
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_BEHAVIOUR_INFORMATION_POLICIES_LOCALDISCUSSION_H_
#define SRC_MAIN_CPP_BEHAVIOUR_INFORMATION_POLICIES_LOCALDISCUSSION_H_

#include "util/RNG.h"

namespace stride {

template <typename PersonType>
class LocalDiscussion {
public:
	static void Update(PersonType* p1, PersonType* p2) {
		if (RNG::GetInstance().NextDouble() < 1.0) {
			p1->Update(p2);
			p2->Update(p1);
		}
	}
};

} /* namespace stride */


#endif /* SRC_MAIN_CPP_BEHAVIOUR_INFORMATION_POLICIES_LOCALDISCUSSION_H_ */
