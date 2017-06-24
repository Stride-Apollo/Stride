#include "RemoteSimulatorSender.h"
#include "mpi.h"

using namespace stride;
using namespace std;

RemoteSimulatorSender::RemoteSimulatorSender(const string& id, const int remote_id): m_count(1), m_id_mpi(remote_id), m_id(id){}

// TODO
// https://stackoverflow.com/questions/14836560/thread-safety-of-mpi-send-using-threads-created-with-stdasync
future<bool> RemoteSimulatorSender::timeStep(){
  std::cout << "Timestep @ RemoteSimulatorSender" << std::endl;
  return async([&](){
      int tag = 4;  // Tag 4 = the remote simulator must execute a timestep and we need to wait until it's done
      std::cout << "Sending message to remote sim @process " << m_id_mpi << std::endl;
      MPI_Send(nullptr, 0, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
      MPI_Recv(nullptr, 0, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      std::cout << "Received message from remote sim @process "<< m_id_mpi << std::endl;
			return true;
		});
}

void RemoteSimulatorSender::welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers){
  int tag = 5;
  ReturnData data{travellers};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

// usually called by the Coordinator
void RemoteSimulatorSender::sendNewTravellers(uint amount, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility){
  int tag = 3;    // Tag of the message (Tag 3 = travellers going to a region issued by the Coordinator)
  std::vector<Simulator::TravellerType> travellers; // Empty vector
  TravelData data {travellers, amount, days, m_id, destination_district, destination_facility};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::returnForeignTravellers(){
  int tag = 6;
  MPI_Send(nullptr, 0, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

// called by the Simulator
void RemoteSimulatorSender::sendNewTravellers(const vector<Simulator::TravellerType>& travellers, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility){
  int tag = 1;    // Tag of the message (Tag 1 = new travellers going to a region)
  uint amount = travellers.size();
  TravelData data {travellers, amount, days, m_id, destination_district, destination_facility};
  // TODO maybe not destination_sim_id as destination for MPI related messages?
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::returnForeignTravellers(const pair<vector<uint>, vector<Health>>& travellers, const string& home_sim_id){
  int tag = 2;    // Tag of the message (Tag 2 = travellers returning home)
  ReturnData data {travellers};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}
