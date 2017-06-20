#include "RemoteSimulatorSender.h"
#include "mpi.h"

using namespace stride;
using namespace std;

RemoteSimulatorSender::RemoteSimulatorSender(Simulator* sim): AsyncSimulator(sim){
  m_sim->setAsyncSimulator(this);
}

// TODO
future<bool> RemoteSimulatorSender::timeStep(){}

void RemoteSimulatorSender::sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility){
  int count = 1;  // The count of elements in the data buffer
  int tag = 1;    // Tag of the message (Tag 1 = new travellers going to a region)
  // TravelData data {amount, days, destination_district, destination_facility};
  TravelData data;
  // TODO maybe not destination_sim_id as destination for MPI related messages?
  MPI_Send(&data, count, MPI_INT, destination_sim_id, tag, MPI_COMM_WORLD);
  m_sim->sendNewTravellers(amount, days, destination_sim_id, destination_district, destination_facility);
}

void RemoteSimulatorSender::welcomeHomeTravellers(const pair<vector<uint>, vector<Health> >& travellers){
}

void RemoteSimulatorSender::sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, uint destination_sim_id, string destination_district, string destination_facility){
  int count = 1;  // The count of elements in the data buffer
  int tag = 1;    // Tag of the message (Tag 1 = new travellers going to a region)
  uint amount = travellers.size();
  // TODO check if amount is correct
  TravelData data {travellers, amount, days, destination_district, destination_facility};
  // TODO maybe not destination_sim_id as destination for MPI related messages?
  MPI_Send(&data, count, MPI_INT, destination_sim_id, tag, MPI_COMM_WORLD);
}
