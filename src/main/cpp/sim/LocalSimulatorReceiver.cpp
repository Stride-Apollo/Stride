#include "LocalSimulatorReceiver.h"
#include "pop/Population.h"
#include "pop/Traveller.h"
#include "core/Cluster.h"
#include "util/GeoCoordinate.h"
#include <memory>
#include <random>

using namespace stride;
using namespace std;
using namespace util;

LocalSimulatorReceiver::LocalSimulatorReceiver(Simulator* sim)
	: AsyncSimulatorReceiver(sim) {
		m_sim->setReceiver(this);
	}

future<bool> LocalSimulatorReceiver::timeStep() {
	return async([&](){
		m_sim->timeStep();
		this->notify(*this);
		return true;
	});
}

bool LocalSimulatorReceiver::hostTravellers(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) {
	m_sim->hostTravellers(travellers, days, destination_district, destination_facility);
	return true;
}

bool LocalSimulatorReceiver::returnTravellers(const vector<uint>& travellers_indices, const vector<Health>& health_status) {
	m_sim->returnHomeTravellers(travellers_indices, health_status);
	return true;
}

void LocalSimulatorReceiver::sendTravellersAway(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) {
	m_sim->sendTravellersAway(amount, days, destination_sim_id, destination_district, destination_facility);
}

void LocalSimulatorReceiver::sendBackForeignTravellers() {
	m_sim->returnForeignTravellers();
}
