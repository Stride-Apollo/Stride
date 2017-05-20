#pragma once

#include <future>

#include "AsyncSimulator.h"
#include "Simulator.h"
#include "util/SimplePlanner.h"
#include "pop/Traveller.h"
#include "util/Subject.h"
#include "checkpointing/Saver.h"
#include "checkpointing/Loader.h"

namespace stride {

class Coordinator;

using namespace std;
using namespace util;

class LocalSimulatorAdapter : public AsyncSimulator, public Subject<LocalSimulatorAdapter> {
public:
	/// The constructor, this adapter will control one simulator
	LocalSimulatorAdapter(Simulator* sim);

	/// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
	virtual future<bool> timeStep() override;

	/// Send travellers to the destination region
	/// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
	/// @argument amount: the amount of travellers to be sent
	/// @argument days: how long these people will be gone
	/// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual vector<unsigned int> sendTravellersAway(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) override;

	virtual vector<unsigned int> sendTravellersHome() override;

	// TODO move two functions below to a receiver?

	/// Receive travellers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	/// TODO: future return value?
	virtual bool hostTravellers(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) override;

	/// Return travellers
	/// @argument travellers_indices: contains the indices (in the m_population->m_original vector) of the returning people
	/// @argument health_status: The Health of the returning people (equal size as travellers_indices, health_status.at(i) belongs to travellers_indices.at(i))
	/// TODO: future return value?
	virtual bool returnTravellers(const vector<uint>& travellers_indices, const vector<Health>& health_status) override;

	const Simulator& getSimulator() const {return *m_sim;}

private:
	void returnTraveller(Simulator::TravellerType& traveller);

	friend class Coordinator;
	friend class Saver;
	friend class Loader;
};

}
