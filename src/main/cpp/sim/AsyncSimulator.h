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
#include "Simulator.h"

namespace stride {

using namespace std;
using namespace util;

class AsyncSimulator {
public:
	AsyncSimulator(Simulator* sim): m_sim(sim) {}

	void setId(uint id) {m_id = id; m_sim->setId(m_id);}
	
	uint getId() const {return m_id;}

	/// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
	virtual future<bool> timeStep() = 0;

	virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health> >& travellers) = 0;

	/// Receive travellers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual void hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) = 0;

	/// Commands to send an amount of travellers to another region
	/// The Simulator will have to take action, select travellers, and indicate which travellers he chose
	/// @argument amount: the amount of travellers to be sent
	/// @argument days: how long these people will be gone
	/// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual void sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) = 0;

	/// Return foreign people that would return today, signals the Simulator to return today's travellers
	virtual void returnForeignTravellers() = 0;

	virtual ~AsyncSimulator() {};

	AsyncSimulator(uint seed = rand()) { std::mt19937 m_rng (seed);}

protected:
	uint m_id = 0;		///< The id of this simulator
	Simulator* m_sim = nullptr;	///< The controlled Simulator

private:
	/// Send specifically chosen travellers to the destination region
	/// This function is used by the Simulator to give the signal to send people
	virtual void sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, uint destination_sim_id, string destination_district, string destination_facility) = 0;

	/// Send foreign travellers to the original region
	/// This function is used by the Simulator to give the signal to send people back
	virtual void returnForeignTravellers(const pair<vector<uint>, vector<Health> >& travellers, uint home_sim_id) = 0;

	friend class Simulator;
};

}
