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
  RemoteSimulatorSender(const int remote_id);
  ~RemoteSimulatorSender() = default;

  /// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
  virtual future<bool> timeStep() override;

  virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers) override;

  /// The hosting is done @Receiver side
  virtual void hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) override {};

  /// Send travellers to the destination region
  /// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
  /// @argument amount: the amount of travellers to be sent
  /// @argument days: how long these people will be gone
  /// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
  /// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
  /// @argument destination_facility: The name of the facility / airport e.g. "ANR"
  virtual void sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) override;

  virtual void returnForeignTravellers() override;

private:
  int m_count; // The count of elements in the databuffer

  /// Send travellers to the destination region
	/// This function is used by the Simulator to give the signal to send people
	virtual void sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, uint destination_sim_id, string destination_district, string destination_facility) override;

  /// Send foreign travellers to the original region
	/// This function is used by the Simulator to give the signal to send people
	virtual void returnForeignTravellers(const pair<vector<uint>, vector<Health> >& travellers, uint home_sim_id) override;

  friend class Simulator;
  friend class Coordinator;
};
}
