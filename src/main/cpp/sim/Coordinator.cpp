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
	std::cout << "Timestep @ Coordinator (#simulators = " << m_sims.size() <<")\n";
	vector<future<bool>> fut_results;

	// Run the simulator for the day
	for (AsyncSimulator* sim: m_sims) {
		std::cout << "Before\n";
		// fut_results.push_back(sim->timeStep());
		std::cout << "After\n";
	}

	future_pool(fut_results);

	// Give each simulator a planning containing todays travellers
	// The simulators will exchange travellers
	// TODO fix the thing with this dynamic cast

	// TODO temporary change for MPI
	// auto some_sim = dynamic_cast<LocalSimulatorAdapter*>(m_sims.at(0));
	auto some_sim = dynamic_cast<RemoteSimulatorSender*>(m_sims.at(0));
	std::cout << "Before cur day\n";
	// uint current_day = some_sim->m_sim->getCalendar().getDayOfTheWeek();
	uint current_day = 1;
	std::cout << "After cur day\n";

	for (uint i = 0; i < m_traveller_schedule[current_day].size(); ++i) {
		// TODO multithreaded, check sim index out of range
		Flight& new_flight = m_traveller_schedule[current_day].at(i);

		// For now, just skip those out-of-bounds simulators
		if (new_flight.m_destination_sim >= m_sims.size() || new_flight.m_source_sim >= m_sims.size()) {
			continue;
		}
		m_sims.at(new_flight.m_source_sim)->sendNewTravellers(new_flight.m_amount,
															new_flight.m_duration,
															m_sims.at(new_flight.m_destination_sim)->getId(),
															new_flight.m_district,
															new_flight.m_facility);
	}
}
