#include "Coordinator.h"
#include "util/async.h"

using namespace stride;
using namespace util;
using namespace std;

void Coordinator::timeStep() {
	static bool send = true;
	vector<future<bool>> fut_results;

	// Run the simulator for the day
	for (AsyncSimulator* sim: m_sims) {
		fut_results.push_back(sim->timeStep());
	}
	future_pool(fut_results);

	// Give each simulator a planning containing todays travellers
	// The simulators will exchange travellers
	// TODO not hardcoded
	if (send) {
		fut_results.clear();
		for (uint i = 0; i < m_sims.size(); ++i) {
			fut_results.push_back(m_sims.at(i)->sendTravellers(1, 5, m_sims.at(i % m_sims.size())));
		}
	}

	send = false;
}
