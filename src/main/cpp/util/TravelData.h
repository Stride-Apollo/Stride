#pragma once

namespace stride{
namespace util{

struct travelData{
  travelData(uint amount, uint days, string destination_district, string destination_facility):
    m_amount(amount), m_days(days), m_destination_district(destination_district), m_destination_facility(destination_facility) {};
  uint m_amount;
  uint m_days;
  string m_destination_district;
  string m_destination_facility;
  
};

}
}
