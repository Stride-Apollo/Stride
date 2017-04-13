#pragma once

#include "Person.h"

namespace stride {

// Perhaps a solution based on inheritance would have 'looked' nicer
// However, since people 'become' travellers at random, that would have
// been very difficult to achieve (and either hacky or inefficient, too)
template <class PersonType>
class Traveller {
public:
	using uint = unsigned int;
	Traveller(uint days_travelling, PersonType* person)
		: m_days_travelling(days_travelling), m_home_id(person->m_id), m_home_household_id(person->m_household_id), m_home_work_id(person->m_work_id),
		m_home_primary_community_id(person->m_primary_community_id), m_home_secondary_community_id(person->m_secondary_community_id),
		m_person(person) {}

	PersonType* getPerson() const {
		return m_person;
	}

	uint getDaysTravelling() {
		return m_days_travelling;
	}

	void resetPerson() {
		// TODO
	}

private:
	uint m_days_travelling;

	uint m_home_id;						///< The home personal id
	uint m_home_household_id;			///< The home household id
	uint m_home_work_id;				///< The home workplace id
	uint m_home_primary_community_id;   ///< The home primary community id
	uint m_home_secondary_community_id; ///< The home secondary community id
	// TODO: destination city/airport

	PersonType* m_person;
};

}
