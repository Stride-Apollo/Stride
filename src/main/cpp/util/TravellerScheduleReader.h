#pragma once

#include <string>
#include <vector>
#include <array>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;

namespace stride {

class Coordinator;

namespace util {

using uint = unsigned int;

struct Flight {
	Flight(uint source_sim, uint destination_sim, uint amount, uint duration, uint day_of_the_week, string district, string facility)
		: m_source_sim(source_sim),
		m_destination_sim(destination_sim),
		m_amount(amount),
		m_duration(duration),
		m_day_of_the_week(day_of_the_week),
		m_district(district),
		m_facility(facility)
		{}
	~Flight() {}

	uint m_source_sim;
	uint m_destination_sim;
	uint m_amount;
	uint m_duration;
	uint m_day_of_the_week;	/// The current day of the week (0 (Sunday), ..., 6 (Saturday))
	string m_district;
	string m_facility;

};

bool operator==(const Flight& flight1, const Flight& flight2);

using Schedule = array<vector<Flight>, 7>;

class TravellerScheduleReader {
public:
	/// Expects a worthy path
	Schedule readSchedule(string filename);

private:
	void parseTree(string filename);

	Flight parseFlight(boost::property_tree::ptree& node) const;

	boost::property_tree::ptree m_pt;
};

}
}