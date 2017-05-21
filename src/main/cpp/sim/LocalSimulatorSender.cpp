#include "LocalSimulatorSender.h"
#include "AsyncSimulatorReceiver.h"
#include "pop/Population.h"
#include "pop/Traveller.h"
#include "core/Cluster.h"
#include "util/GeoCoordinate.h"
#include <memory>
#include <random>

using namespace stride;
using namespace std;
using namespace util;

LocalSimulatorSender::LocalSimulatorSender(Simulator* sim)
	: AsyncSimulatorSender(sim) {}

void LocalSimulatorSender::sendTravellersAway(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) {
	m_receiver->hostTravellers(travellers, days, destination_district, destination_facility);
}

void LocalSimulatorSender::sendTravellersHome(const pair<vector<uint>, vector<Health> >& travellers) {
	m_receiver->returnTravellers(travellers.first, travellers.second);
}