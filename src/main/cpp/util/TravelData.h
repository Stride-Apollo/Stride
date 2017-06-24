#pragma once

#include <vector>
#include <string>
#include <utility>

#include "sim/Simulator.h"

namespace stride{
namespace util{

using namespace std;

struct TravelData{
  TravelData() = default;
  TravelData(vector<Simulator::TravellerType> travellers, uint amount, uint days, const string& destination_sim, const string& destination_district, const string& destination_facility):
    m_amount(amount), m_days(days), m_destination_simulator(destination_sim), m_destination_district(destination_district), m_destination_facility(destination_facility), m_travellers(travellers){};

  uint m_amount;
  uint m_days;
  string m_destination_simulator;
  string m_destination_district;
  string m_destination_facility;
  vector<Simulator::TravellerType> m_travellers;
};

// Datastruct that contains information about the travellers returning to home
struct ReturnData{
  ReturnData() = default;
  ReturnData(pair<vector<uint>, vector<Health>> travellers): m_travellers(travellers) {};

  pair<vector<uint>, vector<Health>> m_travellers;
};

}
}
