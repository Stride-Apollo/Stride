#pragma once

#include "behaviour/belief_data/ThresholdData.h"
#include "core/Health.h"

namespace stride {

/// Forward declaration of class Person
template<typename BehaviourPolicy, typename BeliefPolicy>
class Person;

template<bool threshold_infected, bool threshold_adopted>
class Threshold {
public:
	using Data = ThresholdData;

	static void initialize(Data& belief_data, double risk_averseness) {
		if (threshold_infected) {
			belief_data.setThresholdInfected(1 - risk_averseness);
		}
		if (threshold_adopted) {
			belief_data.setThresholdAdopted(1 - risk_averseness);
		}
	}

	static void update(Data& belief_data, Health& health_data) {}

	template<typename BehaviourPolicy>
	static void
	update(Data& belief_data, const Person<BehaviourPolicy, Threshold<threshold_infected, threshold_adopted>>* p) {
		belief_data.contact<BehaviourPolicy, Threshold<threshold_infected, threshold_adopted>>(p);
	}

	static bool hasAdopted(const Data& belief_data) {
		if (threshold_infected) {
			if (belief_data.getFractionInfected() > belief_data.getThresholdInfected()) {
				return true;
			}
		}
		if (threshold_adopted) {
			if (belief_data.getFractionAdopted() > belief_data.getThresholdAdopted()) {
				return true;
			}
		}

		return false;
	}

};

/// Explicit instantiations in .cpp file
extern template
class Threshold<true, false>;

extern template
class Threshold<false, true>;

extern template
class Threshold<true, true>;

}
