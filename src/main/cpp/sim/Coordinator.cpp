#include "Coordinator.h"
#include "util/async.h"
#include "util/TravellerScheduleReader.h"
#include "calendar/Calendar.h"
#include "sim/LocalSimulatorAdapter.h"
#include "sim/RemoteSimulatorSender.h"

#include <vector>

using namespace stride;
using namespace util;
using namespace std;

void Coordinator::timeStep() {
	vector<future<bool>> fut_results;

	// Run the simulator for the day
	for (auto& it: m_sims) {
		fut_results.push_back(it.second->timeStep());
	}
	future_pool(fut_results);

	int weekday = m_calendar.getDayOfTheWeek();

	/*
	for (uint i = 0; i < m_traveller_schedule.at(weekday).size(); ++i) {
		// TODO multithreaded, check sim index out of range
		Flight& new_flight = m_traveller_schedule[weekday].at(i);

		// For now, just skip those out-of-bounds simulators
		if (! (new_flight.m_destination_sim >= m_sims.size() && new_flight.m_source_sim >= m_sims.size())) {
			m_sims.at(new_flight.m_source_sim)->sendNewTravellers(new_flight.m_amount,
																new_flight.m_duration,
																m_sims.at(new_flight.m_destination_sim)->getId(),
																new_flight.m_district,
																new_flight.m_facility);
		}
	}*/

	m_calendar.advanceDay();
}
