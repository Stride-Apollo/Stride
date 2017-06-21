#pragma once

#include <future>
#include <map>
#include <iostream>

#include "AsyncSimulator.h"
#include "Simulator.h"
#include "util/Subject.h"
#include "util/SimplePlanner.h"
#include "pop/Traveller.h"
#include "util/Subject.h"
// #include "checkpointing/Saver.h"
// #include "checkpointing/Loader.h"

namespace stride {

class Coordinator;
class ClusterSaver;

using namespace std;
using namespace util;

class LocalSimulatorAdapter : public AsyncSimulator, public Subject<LocalSimulatorAdapter> {
public:
	/// The constructor, this adapter will control one simulator
	LocalSimulatorAdapter(Simulator* sim);

	void setCommunicationMap(const std::map<uint, AsyncSimulator*>& adapters) {m_adapters = adapters;}

	virtual future<bool> timeStep() override;

	virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health> >& travellers) override;

	/// Receive travellers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	/// TODO: future return value?
	virtual void hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) override;

	/// Send travellers to the destination region
	/// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
	/// @argument amount: the amount of travellers to be sent
	/// @argument days: how long these people will be gone
	/// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual void sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) override;

	virtual void returnForeignTravellers() override;

	const Simulator& getSimulator() const {return *m_sim;}

private:
	std::map<uint, AsyncSimulator*> m_adapters;	///< A map that contains the interfaces of the other simulators

	/// Send travellers to the destination region
	/// This function is used by the Simulator to give the signal to send people
	virtual void sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, uint destination_sim_id, string destination_district, string destination_facility) override;

	/// Send foreign travellers to the original region
	/// This function is used by the Simulator to give the signal to send people
	virtual void returnForeignTravellers(const pair<vector<uint>, vector<Health> >& travellers, uint home_sim_id) override;

	friend class Simulator;
	friend class Coordinator;
	friend class ClusterSaver;
	friend class Saver;
	friend class Loader;
};

}
