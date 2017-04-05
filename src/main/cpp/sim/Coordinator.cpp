#include "Coordinator.h"
#include "util/async.h"

using namespace stride;
using namespace util;
using namespace std;

void Coordinator::timeStep() {
	vector<future<bool>> fut_results;
	for (AsyncSimulator* sim: m_sims) {
		fut_results.push_back(sim->timeStep());
	}
	future_pool(fut_results);
}
