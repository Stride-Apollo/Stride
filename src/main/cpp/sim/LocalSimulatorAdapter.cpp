#include "LocalSimulatorAdapter.h"
#include "pop/Population.h"
#include "pop/Traveller.h"
#include "core/Cluster.h"
#include "util/RNGPicker.h"
#include "util/GeoCoordinate.h"
#include <memory>
#include <random>

using namespace stride;
using namespace std;
using namespace util;

LocalSimulatorAdapter::LocalSimulatorAdapter(Simulator* sim)
	: m_sim(sim) {

	// Get the next id for new travellers
	uint max_id = 0;
	const Population& population = *(m_sim->m_population.get());
	for (auto& person: population) {
		if (person.getId() > max_id) {
			max_id = person.getId();
		}
	}

	m_next_id = max_id + 1;

	// Get the new household id for travellers
	max_id = 0;
	for (auto& hh: m_sim->m_households) {
		if (hh.getId() > max_id) {
			max_id = hh.getId();
		}
	}

	m_next_hh_id = max_id + 1;
}

future<bool> LocalSimulatorAdapter::timeStep() {
	// Get the people that return home today (according to the planner in the population of this simulator)
	SimplePlanner<Simulator::TravellerType>::Block* returning_people = m_planner.getModifiableDay(0);

	// Make sure the "home stats" of the person are back
	for (auto it = returning_people->begin(); it != returning_people->end(); ++it) {
		Simulator::PersonType* returning_person = (**it).getNewPerson();

		auto work_index = returning_person->m_work_id;
		auto prim_comm_index = returning_person->m_primary_community_id;
		auto sec_comm_index = returning_person->m_secondary_community_id;

		// TODO check for out of range stuff

		m_sim->m_work_clusters.at(work_index).removePerson(returning_person->m_id);
		m_sim->m_primary_community.at(prim_comm_index).removePerson(returning_person->m_id);
		m_sim->m_secondary_community.at(sec_comm_index).removePerson(returning_person->m_id);


		(**it).resetPerson();
	}

	m_planner.nextDay();
	m_sim->m_population->m_visitors.nextDay();
	
	return async([&](){m_sim->timeStep(); return true;});
}

bool LocalSimulatorAdapter::host(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) {
	// Planning:
		// Step 1: No notion of cities / airports, no return of people
			// set family id to 0
			// set random work id
			// set random community id's
		// Step 2:
			// Return the travellers back home (except for United Airlines' passengers, they don't get to travel anymore)
		// >>>>> Step 3: Notion of cities / airports
			// set non-random work id
			// set non-random community id's
		// Step 4:
			// add sphere of influence for airports
	GeoCoordinate facility_location;
	bool found_airport = false;

	for (auto& district: m_sim->m_districts) {
		if (district.getName() == destination_district && district.hasFacility(destination_facility)) {
			found_airport = true;
			facility_location = district.getLocation();
			break;
		}
	}

	if (!found_airport) {
		return false;
	}

	// So that the addresses don't break, reserve the space needed in the vector
	m_sim->m_population.get()->m_visitors.getDay(days);
	m_sim->m_population.get()->m_visitors.getModifiableDay(days)->reserve(travellers.size());

	for (const Simulator::TravellerType& traveller: travellers) {

		// Choose the clusters the traveller will reside in
		uint work_index = chooseCluster(facility_location, m_sim->m_work_clusters);
		uint prim_comm_index = chooseCluster(facility_location, m_sim->m_primary_community);
		uint sec_comm_index = chooseCluster(facility_location, m_sim->m_secondary_community);

		if (work_index == m_sim->m_work_clusters.size()
			|| prim_comm_index == m_sim->m_primary_community.size()
			|| sec_comm_index == m_sim->m_secondary_community.size()) {

			// TODO exception
			cerr << "Failed to find cluster for traveller\n";
		}

		// Get the ID's of these workplaces		
		uint work_id = m_sim->m_work_clusters.at(work_index).getId();
		uint primary_community_id = m_sim->m_primary_community.at(prim_comm_index).getId();
		uint secondary_community_id = m_sim->m_secondary_community.at(sec_comm_index).getId();

		// Make the person
		uint start_infectiousness = traveller.getOldPerson()->getHealth().getStartInfectiousness();
		uint start_symptomatic = traveller.getOldPerson()->getHealth().getStartSymptomatic();

		// Note: the "ID" given to the constructor of a person is actually an index!
		Simulator::PersonType new_person = Simulator::PersonType(m_next_id, traveller.getOldPerson()->getAge(), m_next_hh_id, 0,
																	work_index, prim_comm_index, sec_comm_index,
																	start_infectiousness, start_symptomatic,
																	traveller.getOldPerson()->getHealth().getEndInfectiousness() - start_infectiousness,
																	traveller.getOldPerson()->getHealth().getEndSymptomatic() - start_symptomatic);
		new_person.getHealth() = traveller.getOldPerson()->getHealth();

		
		// Add the person to the planner and set him on "vacation mode" in his home region
		m_sim->m_population->m_visitors.add(days, new_person);
		Simulator::TravellerType new_traveller = Simulator::TravellerType(traveller.getOldPerson(), m_sim->m_population->m_visitors.getModifiableDay(days)->back().get());
		new_traveller.getOldPerson()->m_is_on_vacation = true;
		new_traveller.getNewPerson()->m_is_on_vacation = false;
		m_planner.add(days, new_traveller);

		// Get a pointer to the person you just made
		Simulator::PersonType* person = m_sim->m_population->m_visitors.getModifiableDay(days)->back().get();

		// Add the person to the clusters
		m_sim->m_work_clusters.at(work_index).addPerson(person);
		m_sim->m_primary_community.at(prim_comm_index).addPerson(person);
		m_sim->m_secondary_community.at(sec_comm_index).addPerson(person);

		++m_next_id;
		++m_next_hh_id;
	}

	return true;
}

bool LocalSimulatorAdapter::returnHome(const vector<Simulator::TravellerType>& travellers) {
	// TODO remove?
	// for (auto& traveller: travellers) {
	// 	for (auto& person: m_sim->m_population.get()->m_original) {
	// 		if (traveller.getPerson() == &person) {

	// 		}
	// 	}
	// }

	return true;
}

vector<unsigned int> LocalSimulatorAdapter::sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim, string destination_district, string destination_facility) {
	list<Simulator::PersonType*> working_people;
	vector<unsigned int> people_id_s;

	// Get the working people
	Population& population = *(m_sim->m_population.get());
	for (uint i = 0; i < population.m_original.size(); ++i) {
		Simulator::PersonType& person = population.m_original.at(i);
		if (person.m_work_id != 0 && !person.m_is_on_vacation) {
			working_people.push_back(&person);
		}
	}

	if (amount > working_people.size()) {
		// TODO throw exception
		cout << "Warning, more people to send than actual people in region.\n";
	}

	vector<Simulator::TravellerType> chosen_people;
	//chosen_people.reserve(amount);

	while (chosen_people.size() != amount) {
		// Randomly generate an index in working_people
		uniform_int_distribution<unsigned int> dist(0, working_people.size() - 1);
		unsigned int index = dist(m_rng);

		// Get the person to be sent
		Simulator::PersonType* person = *(next(working_people.begin(), index));
		// Note: don't set vacation on true, the target simulator will do this for you (if he can house the person)

		// Make the sender and make sure he can't be sent twice
		Simulator::TravellerType new_traveller = Simulator::TravellerType(person, nullptr);
		chosen_people.push_back(new_traveller);
		working_people.erase(next(working_people.begin(), index));
		people_id_s.push_back(person->m_id);

	}

	destination_sim->host(chosen_people, days, destination_district, destination_facility);
	return people_id_s;
}