/*
 * ThresholdData.cpp
 *
 *  Created on: May 8, 2017
 *      Author: elise
 */

#include "ThresholdData.h"
#include "pop/Person.h"

namespace stride {

template <typename BehaviourPolicy, typename BeliefPolicy>
void ThresholdData::contact(const Person<BehaviourPolicy, BeliefPolicy>* p) {
	m_num_contacts++;
	if (p->getHealth().isSymptomatic()) {
		m_num_contacts_infected++;
	}
	const auto other_belief_data = p->getBeliefData();
	if (BeliefPolicy::hasAdopted(other_belief_data)) {
		m_num_contacts_adopted++;
	}
}

template void ThresholdData::contact<Vaccination<Threshold<true, false> >, Threshold<true, false> >(const Person<Vaccination<Threshold<true, false> >, Threshold<true, false> >* p);
template void ThresholdData::contact<Vaccination<Threshold<true, false> >, Threshold<false, true> >(const Person<Vaccination<Threshold<true, false> >, Threshold<false, true> >* p);
template void ThresholdData::contact<Vaccination<Threshold<true, false> >, Threshold<true, true> >(const Person<Vaccination<Threshold<true, false> >, Threshold<true, true> >* p);


}
