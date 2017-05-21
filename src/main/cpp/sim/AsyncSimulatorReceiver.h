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

class AsyncSimulatorReceiver {
public:
	AsyncSimulatorReceiver(Simulator* sim): m_sim(sim) {}

	void setId(uint id) {m_id = id;}
	
	uint getId() const {return m_id;}

	/// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
	virtual future<bool> timeStep() = 0;

	/// Receive travellers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	/// TODO: future return value?
	virtual bool hostTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) = 0;

	/// Return travellers
	/// @argument travellers_indices: contains the indices (in the m_population->m_original vector) of the returning people
	/// @argument health_status: The Health of the returning people (equal size as travellers_indices, health_status.at(i) belongs to travellers_indices.at(i))
	/// TODO: future return value?
	virtual bool returnTravellers(const vector<uint>& travellers_indices, const vector<Health>& health_status) = 0;

	/// Send travellers to the destination region
	/// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
	/// @argument amount: the amount of travellers to be sent
	/// @argument days: how long these people will be gone
	/// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual void sendTravellersAway(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) = 0;

	virtual void sendBackForeignTravellers() = 0;

	virtual ~AsyncSimulatorReceiver() {};

	AsyncSimulatorReceiver(uint seed = rand()) { std::mt19937 m_rng (seed);}

protected:
	uint m_id = 0;		///< The id of this simulator
	Simulator* m_sim;	///< The controlled Simulator
};

}
