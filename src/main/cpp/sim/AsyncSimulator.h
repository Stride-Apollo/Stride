#pragma once

#include <future>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <random>

#include "sim/Simulator.h"
#include "util/GeoCoordinate.h"
#include "core/Cluster.h"
#include "pop/TravellerData.h"

namespace stride {

using namespace std;
using namespace util;

class AsyncSimulator {
public:
	void setId(uint id) {m_id = id;}
	
	uint getId() const {return m_id;}

	/// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
	virtual future<bool> timeStep() = 0;

	/// Receive travelers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual bool host(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) = 0;

	/// Send travellers to the destination region
	/// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
	/// @argument amount: the amount of travellers to be sent
	/// @argument days: how long these people will be gone
	/// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual vector<unsigned int> sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim, string destination_district, string destination_facility) = 0;

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Receive a traveller, get your new IDs of the clusters from the traveller data
	/// @argument traveller_data: contains all data needed to reconstruct a traveller after the simulator was restarted using HDF5
	/// @argument traveller: not sure what to do here, i used this for shared memory so that you can get the pointer to the home traveller
	virtual bool forceHost(const Simulator::TravellerType& traveller, const TravellerData& traveller_data) = 0;

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Does the same as AsyncSimulator::forceReturn except for the fact that the travellers aren't sent back home
	/// Return all data needed to reconstruct travellers after a checkpoint
	virtual vector<TravellerData> getTravellerData() = 0;

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Force send a traveller to a simulator
	/// @argument traveller_data: using the data of some traveller, send him back to the destination simulator
	/// @argument destination_sim: a way of communicating with the destination simulator
	virtual void forceSend(const TravellerData& traveller_data, AsyncSimulator* destination_sim) = 0;

	virtual ~AsyncSimulator() {};

	AsyncSimulator(uint seed = rand()) { std::mt19937 m_rng (seed);}

protected:
	uint m_id = 0;
};

}
