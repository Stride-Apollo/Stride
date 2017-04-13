#pragma once

#include <future>

#include "AsyncSimulator.h"
#include "Simulator.h"

namespace stride {

using namespace std;

class LocalSimulatorAdapter : public AsyncSimulator {
public:
	LocalSimulatorAdapter(Simulator* sim);

	virtual future<bool> timeStep() override;

	// Receive travelers
	virtual bool host(const vector<Simulator::TravellerType>& travellers, uint days);

	// Return these travellers back home (in this simulator instance)
	virtual bool returnHome(const vector<Simulator::TravellerType>& travellers);

	// Send travellers to the destination region
	virtual future<bool> sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim);

private:
	Simulator* m_sim;

	uint m_next_id;
	uint m_next_hh_id;
};

}
