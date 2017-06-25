#include "Coordinator.h"
#include "util/async.h"
#include "util/TravellerScheduleReader.h"
#include "calendar/Calendar.h"
#include "sim/SimulatorStatus.h"

#include <vector>
#include <iostream>

using namespace stride;
using namespace util;
using namespace std;

vector<SimulatorStatus> Coordinator::timeStep() {
	vector<future<SimulatorStatus>> fut_results;

	// Run the simulator for the day
	for (auto& it: m_sims) {
		fut_results.push_back(it.second->timeStep());
	}
	auto results = future_pool(fut_results);


	for (auto& it: m_sims) {
		it.second->returnForeignTravellers();
	}

	int weekday = m_calendar.getDayOfTheWeek();

	for (uint i = 0; i < m_traveller_schedule.at(weekday).size(); ++i) {
		Flight& new_flight = m_traveller_schedule[weekday].at(i);
		try {
			m_sims.at(new_flight.m_source_sim)->sendNewTravellers(new_flight.m_amount,
																  new_flight.m_duration,
																  m_sims.at(new_flight.m_destination_sim)->getName(),
																  new_flight.m_district,
																  new_flight.m_facility);
		} catch (...) {
			// Map out of bounds / wrong schedule results in an exception which is caught and ignored here
			cerr << "\nWarning: travelling from " << new_flight.m_source_sim << " to " << new_flight.m_destination_sim
				 << " failed because one of them doesn't exist.\n";
		}
	}

	m_calendar.advanceDay();
	return results;
}
