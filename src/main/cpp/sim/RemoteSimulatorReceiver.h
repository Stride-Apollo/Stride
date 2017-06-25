#pragma once

#include "Simulator.h"
#include "util/TravelData.h"

using namespace stride;
using namespace util;

namespace stride{

class RemoteSimulatorReceiver{
#ifdef MPI_USED
public:
  RemoteSimulatorReceiver(Simulator* sim): m_listening(true), m_count(1), m_sim(sim) {};
  ~RemoteSimulatorReceiver() = default;

  // Receivers listens to messages on the network (MPI)
  void listen();

  // Stop listening to the network
  void stopListening(){m_listening = false;}

private:
  bool m_listening;
  int m_count;

  Simulator* m_sim;
#endif
#ifndef MPI_USED
public:
  	RemoteSimulatorReceiver(Simulator* sim) {};
	~RemoteSimulatorReceiver() = default;

	void listen() {};
	void stopListening() {};
#endif
};

}
