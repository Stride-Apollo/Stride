/*
 * HealthBehavior.h
 *
 *  Created on: Feb 21, 2017
 *      Author: elise
 */

#pragma once

namespace stride {

template <class InformationPolicy, class BeliefPolicy, class BehaviorPolicy>
class HealthBehavior {
public:
	HealthBehavior();
	virtual ~HealthBehavior();
};

} /* namespace stride */

