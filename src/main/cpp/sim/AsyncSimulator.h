#pragma once

#include <future>

namespace stride {

using namespace std;

class AsyncSimulator {
public:
	// The bool doesn't matter, C++ can't handle void
	virtual future<bool> timeStep() = 0;
};

}
