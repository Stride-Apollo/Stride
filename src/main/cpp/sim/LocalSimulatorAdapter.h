#pragma once

#include <future>

#include "AsyncSimulator.h"
#include "Simulator.h"
#include "util/Subject.h"

namespace stride {

using namespace std;

class LocalSimulatorAdapter : public AsyncSimulator, public util::Subject<LocalSimulatorAdapter> {
public:
	LocalSimulatorAdapter(Simulator* sim);

	virtual future<bool> timeStep() override;

	virtual void host(const vector<Simulator::TravellerType>& travellers) override;

public:
	Simulator* m_sim;
};

}
