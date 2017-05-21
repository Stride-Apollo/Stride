#pragma once

#include <future>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <random>
#include <map>

#include "util/GeoCoordinate.h"
#include "util/SimplePlanner.h"
#include "core/Cluster.h"

namespace stride {

using namespace std;
using namespace util;

class AsyncSimulatorSender {
public:
	AsyncSimulatorSender(Simulator* sim): m_sim(sim) {}

	/// Send travellers to the destination region
	virtual void sendTravellersAway(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) = 0;

	virtual void sendTravellersHome(const pair<vector<uint>, vector<Health> >& travellers) = 0;

	virtual ~AsyncSimulatorSender() {};

	AsyncSimulatorSender(uint seed = rand()) { std::mt19937 m_rng (seed);}

protected:
	Simulator* m_sim;	///< The controlled Simulator
};

}
