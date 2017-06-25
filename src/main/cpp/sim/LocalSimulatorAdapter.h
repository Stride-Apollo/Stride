#pragma once

#include <future>
#include <map>
#include <iostream>
#include <string>

#include "AsyncSimulator.h"
#include "Simulator.h"
#include "SimulatorStatus.h"
#include "util/SimplePlanner.h"
#include "pop/Traveller.h"

#include "util/Subject.h"
#include "core/ClusterType.h"


namespace stride {

class Coordinator;
class ClusterSaver;

template<ClusterType clusterType>
class ClusterCalculator;

using namespace std;
using namespace util;

class LocalSimulatorAdapter : public AsyncSimulator {
public:
	/// The constructor, this adapter will control one simulator
	LocalSimulatorAdapter(shared_ptr<Simulator> sim);

	virtual string getName() const override { return m_sim->getName(); };

	virtual future<SimulatorStatus> timeStep() override;

	virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers) override;

	/// Receive travellers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	/// TODO: future return value?
	virtual void hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, const string& destination_district, const string& destination_facility) override;

	/// Send travellers to the destination region
	/// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
	/// @argument amount: the amount of travellers to be sent
	/// @argument days: how long these people will be gone
	/// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	virtual void sendNewTravellers(uint amount, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility) override;

	virtual void returnForeignTravellers() override;

	const Simulator& getSimulator() const {return *m_sim;}

private:
	Simulator* m_sim = nullptr;

	/// Send travellers to the destination region
	/// This function is used by the Simulator to give the signal to send people
	virtual void sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility) override;

	/// Send foreign travellers to the original region
	/// This function is used by the Simulator to give the signal to send people
	virtual void returnForeignTravellers(const pair<vector<uint>, vector<Health> >& travellers, const string& home_sim_id) override;

	friend class Simulator;
	friend class Coordinator;

	template<ClusterType clusterType>
	friend class ClusterCalculator;
};

}
