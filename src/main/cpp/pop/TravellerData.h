#pragma once

#include "Person.h"
#include "sim/Simulator.h"

namespace stride {

using uint = unsigned int;
/*
 * Class that aids in the integration of multi region and HDF5
*/
struct TravellerData {
	TravellerData(const Simulator::PersonType& home_person, const Simulator::PersonType& travelling_person, uint days_left)
			: m_home_id(home_person.getId()),
			m_home_age(home_person.getAge()),
			m_destination_work_id(travelling_person.getClusterId(ClusterType::Work)),
			m_destination_primary_id(travelling_person.getClusterId(ClusterType::PrimaryCommunity)),
			m_destination_secondary_id(travelling_person.getClusterId(ClusterType::SecondaryCommunity)),
			m_days_left(days_left)	{}

	uint m_home_id;
	uint m_home_age;	// For testing and verification only

	uint m_destination_work_id;
	uint m_destination_primary_id;
	uint m_destination_secondary_id;
	uint m_days_left;
};

}