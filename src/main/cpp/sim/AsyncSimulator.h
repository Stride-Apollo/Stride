#pragma once

#include <future>
#include <vector>
#include <string>
#include <cstdlib>

#include "sim/Simulator.h"
#include "util/RNGPicker.h"

namespace stride {

using namespace std;
using namespace util;

class AsyncSimulator {
public:
	// The bool doesn't matter, C++ can't handle void
	// We just need to wait until it is done
	virtual future<bool> timeStep() = 0;

	// Receive travelers
	virtual bool host(const vector<Simulator::TravellerType>& travellers, uint days) = 0;

	// Return these travellers back home (in this simulator instance)
	virtual bool returnHome(const vector<Simulator::TravellerType>& travellers) = 0;

	// Send travellers to the destination region
	virtual future<bool> sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim) = 0;

	virtual ~AsyncSimulator() {};

	AsyncSimulator(string rng_name = "MT19937", uint seed = rand()) {m_rng.set(rng_name, seed);}

protected:
	RNGPicker m_rng;
};

}
