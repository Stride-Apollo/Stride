#pragma once

#include "behaviour/behaviour_policies/AlwaysFollowBeliefs.h"

/*
 * Possible variants:
 * 		+ fraction adopted over entire simulation
 * 		+ with awareness
 * 		+ with history
 * 		+ and off course all combinations
 */
namespace stride {

template <typename BehaviourPolicy, typename BeliefPolicy>
class Person;

template <bool threshold_infected, bool threshold_adopted>
class Threshold;

class ThresholdData {
public:
	/// Default constructor
	ThresholdData():
		m_num_contacts(0U), m_num_contacts_infected(0U), m_num_contacts_adopted(0U),
		m_threshold_infected(1), m_threshold_adopted(1) {}

	void setThresholdInfected(double threshold) {
		m_threshold_infected = threshold;
	}

	double getThresholdInfected() const {
		return m_threshold_infected;
	}

	void setThresholdAdopted(double threshold) {
		m_threshold_adopted = threshold;
	}

	double getThresholdAdopted() const {
		return m_threshold_adopted;
	}

	double getFractionInfected() const {
		if (m_num_contacts == 0) {
			return 0;
		}
		return (double)m_num_contacts_infected / m_num_contacts;
	}

	double getFractionAdopted() const {
		if (m_num_contacts == 0) {
			return 0;
		}
		return (double)m_num_contacts_adopted / m_num_contacts;
	}

	template <typename BehaviourPolicy, typename BeliefPolicy>
	void contact(const Person<BehaviourPolicy, BeliefPolicy>* p);

private:
	unsigned int		m_num_contacts;				///<
	unsigned int		m_num_contacts_infected;	///<
	unsigned int		m_num_contacts_adopted; 	///<

	double 				m_threshold_infected;		///< Fraction of contacts that needs to be infected before person adopts belief.
	double				m_threshold_adopted;		///< Fraction of contacts that needs to have adopted the belief for person to also adopt.

};

extern template void ThresholdData::contact<AlwaysFollowBeliefs, Threshold<true, false> >(const Person<AlwaysFollowBeliefs, Threshold<true, false> >* p);
extern template void ThresholdData::contact<AlwaysFollowBeliefs, Threshold<false, true> >(const Person<AlwaysFollowBeliefs, Threshold<false, true> >* p);
extern template void ThresholdData::contact<AlwaysFollowBeliefs, Threshold<true, true> >(const Person<AlwaysFollowBeliefs, Threshold<true, true> >* p);


//extern template return-type name < argument-list > ( parameter-list ) ;
//extern template ReallyBigFunction<int>();
//extern template void ThresholdData::contact<AlwaysFollowBeliefs, Threshold<true, false> >(const Person<AlwaysFollowBeliefs, Threshold<true, false> >);
//extern template<AlwaysFollowBeliefs, Threshold<true, false> >
//void ThresholdData::contact(const Person<AlwaysFollowBeliefs, Threshold<true, false> >);

}
