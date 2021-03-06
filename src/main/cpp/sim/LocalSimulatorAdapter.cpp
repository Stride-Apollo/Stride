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

LocalSimulatorAdapter::LocalSimulatorAdapter(shared_ptr<Simulator> sim)
		: m_sim(sim.get()) {}

future<SimulatorStatus> LocalSimulatorAdapter::timeStep() {
	return async([&]() {
		return m_sim->timeStep();
	});
}

void LocalSimulatorAdapter::welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers) {
	m_sim->welcomeHomeTravellers(travellers.first, travellers.second);
}

void LocalSimulatorAdapter::hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days,
												  const string& destination_district,
												  const string& destination_facility) {
	m_sim->hostForeignTravellers(travellers, days, destination_district, destination_facility);
}

void LocalSimulatorAdapter::sendNewTravellers(uint amount, uint days, const string& destination_sim_id,
											  const string& destination_district, const string& destination_facility) {
	m_sim->sendNewTravellers(amount, days, destination_sim_id, destination_district, destination_facility);
}

void LocalSimulatorAdapter::returnForeignTravellers() {
	m_sim->returnForeignTravellers();
}

void LocalSimulatorAdapter::sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days,
											  const string& destination_sim_id, const string& destination_district,
											  const string& destination_facility) {
	// TODO wtf?
	//m_adapters.at(destination_sim_id)->hostForeignTravellers(travellers, days, destination_district, destination_facility);
}

void LocalSimulatorAdapter::returnForeignTravellers(const pair<vector<uint>, vector<Health>>& travellers,
													const string& home_sim_id) {
	// TODO wtf?
	//m_adapters.at(home_sim_id)->welcomeHomeTravellers(travellers);
}
