/*
 * ThresholdFractionInfected.h
 *
 *  Created on: Mar 19, 2017
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_BEHAVIOR_BELIEF_POLICIES_THRESHOLDINFECTED_H_
#define SRC_MAIN_CPP_BEHAVIOR_BELIEF_POLICIES_THRESHOLDINFECTED_H_


#include "behavior/belief_data/Threshold.h"


class ThresholdInfected {
	using Data = Threshold;

	static bool BelievesIn(Data belief_data, double fraction_infected) {
		return fraction_infected >= belief_data.GetThreshold();
	}
};

#endif /* SRC_MAIN_CPP_BEHAVIOR_BELIEF_POLICIES_THRESHOLDINFECTED_H_ */
