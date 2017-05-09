#pragma once

#include <future>

#include "AsyncSimulator.h"
#include "Simulator.h"
#include "util/SimplePlanner.h"
#include "pop/Traveller.h"

namespace stride {

using namespace std;
using namespace util;

class LocalSimulatorAdapter : public AsyncSimulator {
public:
	LocalSimulatorAdapter(Simulator* sim);

	virtual future<bool> timeStep() override;

	// Receive travelers
	virtual bool host(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) override;

	// Return these travellers back home (in this simulator instance)
	virtual bool returnHome(const vector<Simulator::TravellerType>& travellers) override;

	// Send travellers to the destination region, return a vector with the ID's of the sent people
	virtual vector<unsigned int> sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim, string destination_district, string destination_facility) override;

public:	// TODO make private again
	Simulator* m_sim;
	SimplePlanner<Traveller<Simulator::PersonType> > m_planner;

	uint m_next_id;
	uint m_next_hh_id;


};

}
