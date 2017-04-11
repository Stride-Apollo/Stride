#pragma once

#include <future>
#include <vector>

#include "sim/Simulator.h"

namespace stride {

using namespace std;

class AsyncSimulator {
public:
	// The bool doesn't matter, C++ can't handle void
	// We just need to wait until it is done
	virtual future<bool> timeStep() = 0;

	// No response needed
	// In fact, no communication will happen to return someone home, it just 'happens'
	virtual void host(const vector<Simulator::TravellerType>& travellers) = 0;

	// TODO returnHome(...)

	virtual ~AsyncSimulator() {};
};

}
