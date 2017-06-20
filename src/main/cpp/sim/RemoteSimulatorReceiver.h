#pragma once

#include "Simulator.h"
#include "util/TravelData.h"

using namespace stride;
using namespace util;

namespace stride{

class RemoteSimulatorReceiver{
public:
  RemoteSimulatorReceiver(Simulator* sim): m_sim(sim) {};
  ~RemoteSimulatorReceiver() = default;

  // Receivers listens to messages on the network (MPI)
  void listen();
private:
  Simulator* m_sim;
};

}
