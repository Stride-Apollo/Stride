#pragma once

#include <vector>
#include <string>

#include "sim/Simulator.h"

namespace stride{
namespace util{

using namespace std;

struct TravelData{
  // Default constructor
  TravelData(){};

  TravelData(vector<Simulator::TravellerType> travellers, uint amount, uint days, string destination_district, string destination_facility):
    m_amount(amount), m_days(days), m_destination_district(destination_district), m_destination_facility(destination_facility), m_travellers(travellers){};

  uint m_amount;
  uint m_days;
  string m_destination_district;
  string m_destination_facility;
  vector<Simulator::TravellerType> m_travellers;
};

}
}
