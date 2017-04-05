#include "LocalSimulatorAdapter.h"

using namespace stride;
using namespace std;

LocalSimulatorAdapter::LocalSimulatorAdapter(Simulator* sim)
	: m_sim(sim) {}

future<bool> LocalSimulatorAdapter::timeStep() {
	return async([&](){m_sim->timeStep(); return true;});
}
