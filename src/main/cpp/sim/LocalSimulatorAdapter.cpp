#include "LocalSimulatorAdapter.h"
#include "pop/Population.h"
#include "pop/Traveller.h"
#include "core/Cluster.h"
#include "util/RNGPicker.h"
#include <memory>
#include <random>

using namespace stride;
using namespace std;

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
	return async([&](){m_sim->timeStep(); return true;});
}

bool LocalSimulatorAdapter::host(const vector<Simulator::TravellerType>& travellers, uint days) {
	// Planning:
		// Step 1: No notion of cities / airports, no return of people
			// set family id to 0
			// set random work id
			// set random community id's
		// Step 2:
			// Return the travellers back home (except for United Airlines' passengers, they don't get to travel anymore)
		// Step 3: Notion of cities / airports
			// set non-random work id
			// set non-random community id's
		// Step 4:
			// add sphere of influence for airports

	// TODO choose wisely
	uint work_id = 1000000;
	uint primary_community_id = 2000000;
	uint secondary_community_id = 3000000;

	for (const Simulator::TravellerType& traveller: travellers) {
		uint start_infectiousness = traveller.getPerson()->getHealth().getStartInfectiousness();
		uint start_symptomatic = traveller.getPerson()->getHealth().getStartSymptomatic();
		Simulator::PersonType new_person = Simulator::PersonType(m_next_id, traveller.getPerson()->getAge(), m_next_hh_id, 0,
																	work_id, primary_community_id, secondary_community_id,
																	start_infectiousness, start_symptomatic,
																	traveller.getPerson()->getHealth().getEndInfectiousness() - start_infectiousness,
																	traveller.getPerson()->getHealth().getEndSymptomatic() - start_symptomatic);

		++m_next_id;
		++m_next_hh_id;
		m_sim->m_population.get()->m_visitors.getModifiableDay(days)->push_back(new_person);
	}

	return true;
}

bool LocalSimulatorAdapter::returnHome(const vector<Simulator::TravellerType>& travellers) {
	// async([&](){m_sim->})
	// TODO
	return true;
}

future<bool> LocalSimulatorAdapter::sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim) {
	list<Simulator::PersonType*> working_people;

	// Get the working people
	Population& population = *(m_sim->m_population.get());
	for (auto& person: population) {
		if (person.m_work_id != 0) {
			working_people.push_back(&person);
			break;
		}
	}

	if (amount > working_people.size()) {
		// TODO throw exception
	}

	// TODO choose person
	vector<Simulator::TravellerType> chosen_people;
	chosen_people.reserve(amount);

	while (chosen_people.size() != amount) {
		// Randomly generate an index in working_people
		uniform_int_distribution<int> dist(0, working_people.size() - 1);
		int index = dist(m_rng);

		// Get the person to be sent
		Simulator::PersonType* person = *(next(working_people.begin(), index));
		person->m_is_on_vacation = true;

		// Make the sender and make sure he can't be sent twice
		Simulator::TravellerType new_traveller {days, person};
		chosen_people.push_back(new_traveller);
		working_people.erase(next(working_people.begin(), index));
	}

	destination_sim->host(chosen_people, days);
}