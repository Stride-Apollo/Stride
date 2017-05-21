#pragma once

#include <future>

#include "AsyncSimulatorSender.h"
#include "Simulator.h"
#include "util/SimplePlanner.h"
#include "pop/Traveller.h"
#include "util/Subject.h"
// #include "checkpointing/Saver.h"
// #include "checkpointing/Loader.h"

namespace stride {

class Coordinator;

using namespace std;
using namespace util;

class LocalSimulatorSender : public AsyncSimulatorSender, public Subject<LocalSimulatorSender> {
public:
	/// The constructor, this adapter will control one simulator
	LocalSimulatorSender(Simulator* sim);

	void setReceiver(AsyncSimulatorReceiver* receiver, uint receiver_id) {m_receiver = receiver; m_receiver_id = receiver_id;}

	/// Send travellers to the destination region
	virtual void sendTravellersAway(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) override;

	virtual void sendTravellersHome(const pair<vector<uint>, vector<Health> >& travellers) override;

	const Simulator& getSimulator() const {return *m_sim;}

private:
	AsyncSimulatorReceiver* m_receiver;	///< The receiver to which this sender will send things
	uint m_receiver_id;

	friend class Coordinator;
	friend class Saver;
	friend class Loader;
};

}
