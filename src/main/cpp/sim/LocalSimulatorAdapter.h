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

private:
	Simulator* m_sim;
};

}
