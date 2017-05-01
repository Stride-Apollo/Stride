/*
 * Threshold.h
 *
 *  Created on: Mar 19, 2017
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_BEHAVIOR_BELIEF_DATA_THRESHOLD_H_
#define SRC_MAIN_CPP_BEHAVIOR_BELIEF_DATA_THRESHOLD_H_


class Threshold {

public:
	void SetThreshold(double threshold) {
		m_threshold = threshold;
	}

	double GetThreshold() const {
		return m_threshold;
	}

private:
	double m_threshold;
};


#endif /* SRC_MAIN_CPP_BEHAVIOR_BELIEF_DATA_THRESHOLD_H_ */
