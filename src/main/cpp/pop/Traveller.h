#pragma once

#include "Person.h"

namespace stride {

// Perhaps a solution based on inheritance would have 'looked' nicer
// However, since people 'become' travellers at random, that would have
// been very difficult to achieve (and either hacky or inefficient, too)
template <class PersonType>
class Traveller {
public:
	Traveller(unsigned int days_travelling, unsigned int home, unsigned int home_id)
		: m_days_travelling(days_travelling), m_home(home), m_home_id(home_id) {}

	PersonType* getPerson() const {
		return person;
	}

	unsigned int getDaysTravelling() {
		return m_days_travelling;
	}

private:
	unsigned int m_days_travelling;

	// This is just for debugging purposes and can be removed in the future,
	// since 'returning' a person home does not use any sort of communication.
	// The home simulator simply inserts the person back into the population,
	// and the remote simulator removes that person.
	unsigned int m_home;
	unsigned int m_home_id;

	// TODO: destination city/airport

	PersonType* person;
};

}
