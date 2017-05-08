#include "LocalSimulatorAdapter.h"

using namespace stride;
using namespace std;

LocalSimulatorAdapter::LocalSimulatorAdapter(Simulator* sim)
	: m_sim(sim) {}

future<bool> LocalSimulatorAdapter::timeStep() {
	return async([&](){m_sim->timeStep(); this->notify(*this); return true;});
}

void LocalSimulatorAdapter::host(const vector<Simulator::TravellerType>& travellers) {
	//async([&](){m_sim->})
}
