#include "RemoteSimulatorSender.h"
#include "SimulatorStatus.h"

using namespace stride;
using namespace std;

RemoteSimulatorSender::RemoteSimulatorSender(const string& name, const int remote_id)
        : m_count(1), m_id_mpi(remote_id), m_name(name) {}

future<SimulatorStatus> RemoteSimulatorSender::timeStep(){
  std::cout << "Timestep @ RemoteSimulatorSender" << std::endl;
  return async([&](){
      int tag = 4;  // Tag 4 = the remote simulator must execute a timestep and we need to wait until it's done
      SimulatorStatus data {0,0};
      std::cout << "Sending message to remote sim @process " << m_id_mpi << std::endl;
      MPI_Send(nullptr, 0, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
      MPI_Recv(&data, m_count, m_simulator_status, m_id_mpi, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      std::cout << "Received message from remote sim @process "<< m_id_mpi << std::endl;
			return data;
		});
}

void RemoteSimulatorSender::welcomeHomeTravellers(const pair<vector<uint>, vector<Health>>& travellers){
  int tag = 5;
  ReturnData data{travellers};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::hostForeignTravellers(const vector<stride::Simulator::TravellerType>& travellers, uint days, const string& destination_district, const string& destination_facility){
  int tag = 7;
  uint amount = travellers.size();
  TravelData data {travellers, amount, days, m_name, destination_district, destination_facility};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

// usually called by the Coordinator
void RemoteSimulatorSender::sendNewTravellers(uint amount, uint days, const string& destination_sim_id, const string& destination_district, const string& destination_facility){
  int tag = 3;    // Tag of the message (Tag 3 = travellers going to a region issued by the Coordinator)
  std::vector<Simulator::TravellerType> travellers; // Empty vector
  // TODO replace m_id with destination_sim_id?
  TravelData data {travellers, amount, days, m_name, destination_district, destination_facility};
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
  // TODO replace m_id with destination_sim_id?
  TravelData data {travellers, amount, days, m_name, destination_district, destination_facility};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::returnForeignTravellers(const pair<vector<uint>, vector<Health>>& travellers, const string& home_sim_id){
  int tag = 2;    // Tag of the message (Tag 2 = travellers returning home)
  ReturnData data {travellers};
  MPI_Send(&data, m_count, MPI_INT, m_id_mpi, tag, MPI_COMM_WORLD);
}

void RemoteSimulatorSender::makeSimulatorStatus(){
  MPI_Datatype type[2] = {MPI_INT, MPI_INT};
  /** Number of occurence of each type */
  int blocklen[2] = {1, 1};
  /** Position offset from struct starting address */
  MPI_Aint disp[2];
  disp[0] = offsetof(SimulatorStatus, infected);
  disp[1] = offsetof(SimulatorStatus, adopted);
  /** Create the type */
  MPI_Type_create_struct(2, blocklen, disp, type, &m_simulator_status);
  MPI_Type_commit(&m_simulator_status);
}
