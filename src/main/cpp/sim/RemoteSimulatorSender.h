#pragma once

#include <future>
#include <string>

#include "AsyncSimulator.h"
#include "Simulator.h"
#include "pop/Traveller.h"
#include "util/TravelData.h"

namespace stride{

using namespace std;
using namespace util;

class RemoteSimulatorSender: public AsyncSimulator{
public:
  RemoteSimulatorSender(Simulator* sim);
  ~RemoteSimulatorSender() = default;

  virtual future<bool> timeStep() override;

  virtual void sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) override;

  virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health> >& travellers) override;

private:
  friend class Simulator;
};
}
