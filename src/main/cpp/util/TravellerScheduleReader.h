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
	Flight(uint source_sim, uint destination_sim, uint amount, uint duration, uint day_of_the_week) {}
	~Flight() {}

	uint m_source_sim;
	uint m_destionation_sim;
	uint m_amount;
	uint m_duration;
	uint m_day_of_the_week;	/// The current day of the week (0 (Sunday), ..., 6 (Saturday))

};

using Schedule = array<vector<Flight>, 7>;

class TravellerScheduleReader {
public:
	Schedule parseTree(string filename);

private:
	void readSchedule(string filename);

	Flight parseFlight(boost::property_tree::ptree& node) const;

	boost::property_tree::ptree m_pt;
};

}
}