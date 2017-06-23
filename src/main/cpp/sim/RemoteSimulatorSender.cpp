#include "RemoteSimulatorSender.h"
#include "mpi.h"

using namespace stride;
using namespace std;

RemoteSimulatorSender::RemoteSimulatorSender(const int remote_id): m_count(1){
  this->m_id = remote_id;
}

// TODO
// https://stackoverflow.com/questions/14836560/thread-safety-of-mpi-send-using-threads-created-with-stdasync
future<bool> RemoteSimulatorSender::timeStep(){
  std::cout << "Timestep @ RemoteSimulatorSender" << std::endl;
  return async([&](){
			m_sim->timeStep();
			// this->notify(*this);
			return true;
		});
}

void RemoteSimulatorSender::welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers){
  std::cout << "TODO" << std::endl;
  // m_sim->welcomeHomeTravellers(travellers.first, travellers.second);
}

// usually called by the Coordinator
void RemoteSimulatorSender::sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility){
  int tag = 3;    // Tag of the message (Tag 3 = travellers going to a region issued by the Coordinator)
  std::vector<Simulator::TravellerType> travellers; // Empty vector
  TravelData data {travellers, amount, days, destination_district, destination_facility};
  MPI_Send(&data, m_count, MPI_INT, destination_sim_id, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::returnForeignTravellers(){
  std::cout << "TODO" << std::endl;
  // m_sim->returnForeignTravellers();
}

// called by the Simulator
void RemoteSimulatorSender::sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, uint destination_sim_id, string destination_district, string destination_facility){
  int tag = 1;    // Tag of the message (Tag 1 = new travellers going to a region)
  uint amount = travellers.size();
  TravelData data {travellers, amount, days, destination_district, destination_facility};
  // TODO maybe not destination_sim_id as destination for MPI related messages?
  MPI_Send(&data, m_count, MPI_INT, destination_sim_id, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::returnForeignTravellers(const pair<vector<uint>, vector<Health>>& travellers, uint home_sim_id){
  int tag = 2;    // Tag of the message (Tag 2 = travellers returning home)
  ReturnData data {travellers};
  MPI_Send(&data, m_count, MPI_INT, home_sim_id, tag, MPI_COMM_WORLD);
}
