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
	Traveller(const PersonType& home_person, PersonType* new_person, uint home_sim_id, uint destination_sim_id, uint home_simulator_index)
		: m_home_simulator_id(home_sim_id), m_destination_simulator_id (destination_sim_id),
		m_home_simulator_index(home_simulator_index),
		m_home_person(home_person), m_new_person(new_person) {}

	Traveller(const Traveller& other_traveller)
		: m_home_simulator_id(other_traveller.m_home_simulator_id), m_destination_simulator_id(other_traveller.m_destination_simulator_id),
		m_home_person(other_traveller.m_home_person), m_new_person(other_traveller.m_new_person)
		 {}

	const PersonType& getHomePerson() const {
		return m_home_person;
	}

	PersonType* getNewPerson() const {
		return m_new_person;
	}

	uint getHomeSimulatorId() const {
		return m_home_simulator_id;
	}

	uint getHomeSimulatorIndex() const {
		return m_home_simulator_index;
	}

	uint getDestinationSimulatorId() const {
		return m_destination_simulator_id;
	}

private:
	uint m_home_simulator_id;			///< The id of the home simulator
	uint m_destination_simulator_id;	///< The id of the destination simulator
	uint m_home_simulator_index;		///< The index of the person in the home simulator

	PersonType m_home_person;			///< The person in the region of origin
	PersonType* m_new_person;			///< The person when he travelled to the other region
};

}
