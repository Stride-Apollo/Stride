#pragma once

#include "Person.h"
#include <iostream>
using namespace std;

namespace stride {

// Perhaps a solution based on inheritance would have 'looked' nicer
// However, since people 'become' travellers at random, that would have
// been very difficult to achieve (and either hacky or inefficient, too)

class LocalSimulatorAdapter;

template <class PersonType>
class Traveller {
public:
	using uint = unsigned int;
	Traveller(PersonType* old_person, PersonType* new_person, uint home_sim_id, uint destination_sim_id)
		: m_home_id(old_person->m_id), m_home_household_id(old_person->m_household_id), m_home_work_id(old_person->m_work_id),
		m_home_primary_community_id(old_person->m_primary_community_id), m_home_secondary_community_id(old_person->m_secondary_community_id),
		m_home_simulator_id(home_sim_id), m_destination_simulator_id (destination_sim_id),
		m_old_person(old_person), m_new_person(new_person) {}

	Traveller(const Traveller& other_traveller)
		: m_home_id(other_traveller.m_home_id), m_home_household_id(other_traveller.m_home_household_id), m_home_work_id(other_traveller.m_home_work_id),
		m_home_primary_community_id(other_traveller.m_home_primary_community_id), m_home_secondary_community_id(other_traveller.m_home_secondary_community_id),
		m_old_person(other_traveller.m_old_person), m_new_person(other_traveller.m_new_person),
		m_home_simulator_id(other_traveller.m_home_simulator_id), m_destination_simulator_id(other_traveller.m_destination_simulator_id)
		 {}

	PersonType* getOldPerson() const {
		return m_old_person;
	}

	PersonType* getNewPerson() const {
		return m_new_person;
	}

	uint getHomeSimulatorId() const {
		return m_home_simulator_id;
	}

	uint getDestinationSimulatorId() const {
		return m_destination_simulator_id;
	}

	void resetPerson() {
		// Update the person
		*m_old_person = *m_new_person;

		m_old_person->m_id = m_home_id;
		m_old_person->m_household_id = m_home_household_id;
		m_old_person->m_work_id = m_home_work_id;
		m_old_person->m_primary_community_id = m_home_primary_community_id;
		m_old_person->m_secondary_community_id = m_home_secondary_community_id;
		m_old_person->m_is_on_vacation = false;
	}

private:
	uint m_home_id;						///< The home personal id
	uint m_home_household_id;			///< The home household id
	uint m_home_work_id;				///< The home workplace id
	uint m_home_primary_community_id;   ///< The home primary community id
	uint m_home_secondary_community_id; ///< The home secondary community id
	uint m_home_simulator_id;			///< The id of the home simulator
	uint m_destination_simulator_id;	///< The id of the destination simulator
	// TODO: destination city/airport

	PersonType* m_old_person;			///< The person in the region of origin
	PersonType* m_new_person;			///< The person when he travelled to the other region
};

}
