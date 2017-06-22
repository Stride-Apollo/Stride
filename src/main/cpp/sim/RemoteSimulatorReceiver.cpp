#include "RemoteSimulatorReceiver.h"
#include "mpi.h"

#include <iostream>

using namespace stride;
using namespace std;
using namespace util;

void RemoteSimulatorReceiver::listen(){
  cout << "Listening\n";
  MPI_Status status;
  cout << "Start probe\n";
  int flag = 0;
  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
  cout << "Tag " << status.MPI_TAG << endl;
  cout << "Flag " << flag << endl;
  if (status.MPI_TAG == 1 and flag == 1){
    cout << "Received travellers " << status.MPI_TAG << endl;
    // Tag 1 means travellers from another region (sendNewTravellers @ RemoteSimulatorSender)
    TravelData data;
    MPI_Recv(&data, m_count, MPI_INT, MPI_ANY_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->hostForeignTravellers(data.m_travellers, data.m_days, data.m_destination_district, data.m_destination_facility);
  }
  if (status.MPI_TAG == 2 and flag == 1){
    // Tag 2 means travellers returning home (returnForeignTravellers @ RemoteSimulatorSender)
    ReturnData data;
    MPI_Recv(&data, m_count, MPI_INT, MPI_ANY_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->welcomeHomeTravellers(data.m_travellers.first, data.m_travellers.second);
  }
}
