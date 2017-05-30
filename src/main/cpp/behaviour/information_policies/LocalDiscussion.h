/*
 * LocalDiscussion.h
 *
 *  Created on: May 26, 2017
 *      Author: elise
 */

#pragma once

#include "util/RNG.h"

namespace stride {

template <typename PersonType>
class LocalDiscussion {
public:
	static void update(PersonType* p1, PersonType* p2) {
		if (RNG::getInstance().nextDouble() < 1.0) {
			p1->update(p2);
			p2->update(p1);
		}
	}
};

} /* namespace stride */


