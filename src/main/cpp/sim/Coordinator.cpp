#include "Coordinator.h"
#include "util/async.h"
#include "util/TravellerScheduleReader.h"
#include "calendar/Calendar.h"
#include "sim/LocalSimulatorAdapter.h"

using namespace stride;
using namespace util;
using namespace std;

#include <iostream>

void Coordinator::timeStep() {
	vector<future<bool>> fut_results;

	for (AsyncSimulator* sim: m_sims) {
		sim->returnHome({});
	}

	// Run the simulator for the day
	for (AsyncSimulator* sim: m_sims) {
		fut_results.push_back(sim->timeStep());
	}
	future_pool(fut_results);

	// Give each simulator a planning containing todays travellers
	// The simulators will exchange travellers
	// TODO fix the thing with this dynamic cast
	
	auto some_sim = dynamic_cast<LocalSimulatorAdapter*>(m_sims.at(0));
	uint current_day = some_sim->m_sim->m_calendar->getDayOfTheWeek();

	for (uint i = 0; i < m_traveller_schedule[current_day].size(); ++i) {
		// TODO multithreaded, remove hardcoded district, check sim index ou of range
		Flight& new_flight = m_traveller_schedule[current_day].at(i);
		m_sims.at(new_flight.m_source_sim)->sendTravellers(new_flight.m_amount, new_flight.m_duration, m_sims.at(new_flight.m_destination_sim), "Antwerp", "Airport 1");
	}
}
