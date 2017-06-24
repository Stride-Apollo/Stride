/*
 * HBM.h
 *
 *  Created on: Apr 20, 2017
 *      Author: elise
 */

#pragma once

#include "core/Health.h"
#include "util/RNG.h"
#include "behaviour/belief_data/HBMData.h"

#include <cmath>

namespace stride {

template<typename BehaviourPolicy, typename BeliefPolicy>
class Person;

/*
 * p(behaviour) = OR0 * (OR1^x1 * OR2^x2 * OR3^x3 * OR4^x4)/ (1 + OR0 * (prod ORi^xi))
 */

class HBM {
public:
	using Data = HBMData;

	static void update(Data& belief_data, Health& health_data) {

	}

	template<typename BehaviourPolicy>
	static void update(Data& belief_data, const Person<BehaviourPolicy, HBM>* p) {

	}

	static bool hasAdopted(const Data& belief_data) {
		int perceived_severity = belief_data.GetPerceivedSeverity();
		int perceived_susceptibility = belief_data.GetPerceivedSusceptibility();
		int perceived_benefit = belief_data.GetPerceivedBenefit();
		int perceived_barriers = belief_data.GetPerceivedBarriers();
		double odds = m_odds_ratio_0 * std::pow(m_odds_ratio_severity, perceived_severity)
									 * std::pow(m_odds_ratio_susceptibility, perceived_susceptibility)
									 * std::pow(m_odds_ratio_benefit, perceived_benefit)
									 * std::pow(m_odds_ratio_benefit, perceived_barriers);
		double p_behaviour = odds / (1 + odds);
		return p_behaviour >= 0.5;
	}

private:
	static const double m_odds_ratio_0 = -1;
	static const double m_odds_ratio_severity = 0.626;
	static const double m_odds_ratio_susceptibility = 1.07;
	static const double m_odds_ratio_benefit = 0;
	static const double m_odds_ratio_barriers = -0.4;
};

}

