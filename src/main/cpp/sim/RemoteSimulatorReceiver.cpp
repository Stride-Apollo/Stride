#include "RemoteSimulatorReceiver.h"
#include "mpi.h"

#include <iostream>

using namespace stride;
using namespace std;
using namespace util;

void RemoteSimulatorReceiver::listen(){
  cout << "Listening\n";
  MPI_Status status;
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  cout << "Tag: " << status.MPI_TAG << endl;
  // // Message received
  if (status.MPI_TAG == 1){
    cout << "Received travellers " << status.MPI_TAG << endl;
    // Tag 1 means travellers from another region (sendNewTravellers @ RemoteSimulatorSender)
    TravelData data;
    MPI_Recv(&data, m_count, MPI_INT, MPI_ANY_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->hostForeignTravellers(data.m_travellers, data.m_days, data.m_destination_district, data.m_destination_facility);
  }
  if (status.MPI_TAG == 2 or status.MPI_TAG == 5){
    // Tag 2 means travellers returning home (returnForeignTravellers @ RemoteSimulatorSender)
    ReturnData data;
    MPI_Recv(&data, m_count, MPI_INT, MPI_ANY_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->welcomeHomeTravellers(data.m_travellers.first, data.m_travellers.second);
  }
  if (status.MPI_TAG == 3){
    // Tag 3 means travellers from another region (issued by the Coordinator)
    TravelData data;
    MPI_Recv(&data, m_count, MPI_INT, MPI_ANY_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->sendNewTravellers(data.m_amount, data.m_days, status.MPI_SOURCE, data.m_destination_district, data.m_destination_facility);
  }
  if (status.MPI_TAG == 4) {
    // Tag 4 means that this simulator must execute a timestep
    int data;
    MPI_Recv(&data, m_count, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->timeStep();
  }
  if (status.MPI_TAG == 6){
    // Tag 6 means travellers returning home
    int data;
    MPI_Recv(&data, m_count, MPI_INT, MPI_ANY_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m_sim->returnForeignTravellers();
  }
  if (status.MPI_TAG == 10) { // End listening
    int data;
    MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    std::cout << "No more messages\n";
    return;
  }

  // After message is received and processed, start listening again
  this->listen();
}
