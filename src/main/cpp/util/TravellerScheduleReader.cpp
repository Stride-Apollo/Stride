#include "util/TravellerScheduleReader.h"

#include <fstream>
#include <stdexcept>

using namespace stride;
using namespace util;
using namespace std;

bool stride::util::operator==(const Flight& flight1, const Flight& flight2) {
	return flight1.m_source_sim == flight2.m_source_sim
				&& flight1.m_destination_sim == flight2.m_destination_sim
				&& flight1.m_amount == flight2.m_amount
				&& flight1.m_duration == flight2.m_duration
				&& flight1.m_day_of_the_week == flight2.m_day_of_the_week;
}

Schedule TravellerScheduleReader::readSchedule(string filename) {
	// Read the file
	parseTree(filename);

	Schedule schedule;
	auto schedule_config = m_pt.get_child("travel_schedule");

	// Loop over all flights
	for (auto it = schedule_config.begin(); it != schedule_config.end(); it++) {
		// Parse the flight and add it to the schedule
		Flight new_flight = parseFlight(it->second);
		schedule[new_flight.m_day_of_the_week].push_back(new_flight);
	}
	return schedule;
}

void TravellerScheduleReader::parseTree(string filename) {
	// Parse the file
	try {
		boost::property_tree::read_xml(filename, m_pt);
	} catch(...) {
		throw invalid_argument("In TravellerScheduleReader: Error while parsing.");
	}
}

Flight TravellerScheduleReader::parseFlight(boost::property_tree::ptree& node) const {
	string source_sim = node.get<string>("direction.<xmlattr>.src");
	string destination_sim = node.get<string>("direction.<xmlattr>.dest");

	int amount = node.get<int>("<xmlattr>.amount");
	if (amount < 0) {
		throw invalid_argument("In TravellerScheduleReader: Invalid amount of travellers.");
	}

	int duration = node.get<int>("<xmlattr>.duration");
	if (duration < 0) {
		throw invalid_argument("In TravellerScheduleReader: Invalid duration.");
	}

	int day_of_the_week = node.get<int>("<xmlattr>.day_of_the_week");
	if (day_of_the_week < 0 || day_of_the_week > 6) {
		throw invalid_argument("In TravellerScheduleReader: Invalid day of the week.");
	}

	string district = node.get<string>("arrival.<xmlattr>.district");
	string facility = node.get<string>("arrival.<xmlattr>.facility");

	return Flight(source_sim, destination_sim, uint(amount), uint(duration), uint(day_of_the_week), district, facility);
}