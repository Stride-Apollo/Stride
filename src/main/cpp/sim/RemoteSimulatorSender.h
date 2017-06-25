#pragma once

#include <future>
#include <string>
#include <map>
#ifdef MPI_USED
#include <mpi.h>
#endif
#include "AsyncSimulator.h"
#include "Simulator.h"
#include "pop/Traveller.h"
#include "util/TravelData.h"

namespace stride{

using namespace std;
using namespace util;

class RemoteSimulatorSender: public AsyncSimulator{
#ifdef MPI_USED
public:
  RemoteSimulatorSender(const string& m_name, const int mpi_id);
  ~RemoteSimulatorSender() = default;

  virtual string getName() const override { return m_name; };

  /// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
  virtual future<SimulatorStatus> timeStep() override;

  virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers) override;

  virtual void hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, const string& destination_district, const string& destination_facility) override;

  /// Send travellers to the destination region
  /// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
  /// @argument amount: the amount of travellers to be sent
  /// @argument days: how long these people will be gone
  /// @argument destination_sim: a way of communicating with the destination simulator, this must contain all data to achieve communication
  /// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
  /// @argument destination_facility: The name of the facility / airport e.g. "ANR"
  virtual void sendNewTravellers(uint amount, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility) override;

  virtual void returnForeignTravellers() override;

private:
  int m_count;      // The count of elements in the databuffer (default = 1)
  int m_id_mpi;     // The id which will be used for MPI communication
  string m_name;    // The standard name (string)
  MPI_Datatype m_simulator_status;
  MPI_Datatype m_returning_travellers;

  /// Send travellers to the destination region
  /// This function is used by the Simulator to give the signal to send people
  virtual void sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility) override;

  /// Send foreign travellers to the original region
  /// This function is used by the Simulator to give the signal to send people
  virtual void returnForeignTravellers(const pair<vector<uint>, vector<Health>>& travellers, const string& home_sim_id) override;

  void makeSimulatorStatus();
  void makeTravellersReturningStruct();
  friend class Simulator;
  friend class Coordinator;
#endif
#ifndef MPI_USED
public:
  RemoteSimulatorSender(const string& m_name, const int mpi_id) {}
  ~RemoteSimulatorSender() = default;

  virtual string getName() const override { return ""; };
  virtual future<SimulatorStatus> timeStep() override {return async([&](){return SimulatorStatus(0, 0);});}
  virtual void welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers) override {}
  virtual void hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, const string& destination_district, const string& destination_facility) override {}
  virtual void sendNewTravellers(uint amount, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility) override {}
  virtual void returnForeignTravellers() override {}
#endif
};

}
