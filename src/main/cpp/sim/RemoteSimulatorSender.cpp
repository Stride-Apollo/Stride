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
  int tag = 1;    // Tag of the message
  travelData data {amount, days, destination_district, destination_facility};
  MPI_Send(&data, count, MPI_INT, destination_sim_id, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::welcomeHomeTravellers(const pair<vector<uint>, vector<Health> >& travellers){
}
