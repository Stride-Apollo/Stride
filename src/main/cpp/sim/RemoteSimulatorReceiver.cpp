#include "RemoteSimulatorReceiver.h"
#include "mpi.h"

using namespace stride;
using namespace std;
using namespace util;

void RemoteSimulatorReceiver::hostForeignTravellers(travelData data){
  int count = 1;
  int tag = 1;
  MPI_Recv(&data, count, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  // TODO how to get the correct travellers here
  // m_sim->hostForeignTravellers(data.m_travellers, data.m_days, data.m_destination_district, data.m_destination_facility)
}
