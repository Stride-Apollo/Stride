#include "LocalSimulatorAdapter.h"
#include "pop/Population.h"
#include "pop/Traveller.h"
#include "core/Cluster.h"
#include "util/GeoCoordinate.h"
#include <memory>
#include <random>

using namespace stride;
using namespace std;
using namespace util;

LocalSimulatorAdapter::LocalSimulatorAdapter(Simulator* sim)
	: AsyncSimulator(sim) {}

future<bool> LocalSimulatorAdapter::timeStep() {
	return async([&](){
		m_sim->timeStep();
		this->notify(*this);
		return true;
	});
}

vector<unsigned int> LocalSimulatorAdapter::sendTravellersAway(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) {
	auto travellers = m_sim->pickTravellersToSend(amount, days, destination_sim_id, m_id);

	m_communication_map[destination_sim_id]->hostTravellers(travellers, days, destination_district, destination_facility);

	return vector<unsigned int>();
}

vector<unsigned int> LocalSimulatorAdapter::sendTravellersHome() {
	auto travellers = m_sim->returnForeignTravellers();

	for (auto it = m_communication_map.begin(); it != m_communication_map.end(); ++it) {
		if (it->first != m_id && it->first < travellers.size()) {
			m_communication_map[it->first]->returnTravellers(travellers.at(it->first).first, travellers.at(it->first).second);
		}
	}

	return vector<unsigned int>();
}

bool LocalSimulatorAdapter::hostTravellers(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) {
	m_sim->hostTravellers(travellers, days, destination_district, destination_facility);
	return true;
}

bool LocalSimulatorAdapter::returnTravellers(const vector<uint>& travellers_indices, const vector<Health>& health_status) {
	m_sim->returnHomeTravellers(travellers_indices, health_status);
	return true;
}
