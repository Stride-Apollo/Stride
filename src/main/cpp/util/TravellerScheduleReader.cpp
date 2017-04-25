#include "util/TravellerScheduleReader.h"

#include <fstream>
#include <stdexcept>

using namespace stride;
using namespace util;
using namespace std;

Schedule TravellerScheduleReader::parseTree(string filename) {
	// Read the file
	readSchedule(filename);

	Schedule schedule;
	auto schedule_config = m_pt.get_child("Schedule");

	// Loop over all flights
	for (auto it = schedule_config.begin(); it != schedule_config.end(); it++) {
		if (it->first == "Flight") {
			// Parse the flight and add it to the schedule
			Flight new_flight = parseFlight(it->second);
			schedule[new_flight.m_day_of_the_week].push_back(new_flight);
		} else {
			throw invalid_argument("In TravellerScheduleReader: Unexpected value: " + it->first + ".");
		}
	}

	return schedule;
}

void TravellerScheduleReader::readSchedule(string filename) {
	// Check if the file exists
	ifstream f(filename.c_str());
	if (!f.good()) {
		throw invalid_argument("Invalid schedule file");
	}

	// Parse the file
	try {
		boost::property_tree::read_json(filename, m_pt);
	} catch(...) {
		throw invalid_argument("In TravellerScheduleReader: Error while parsing.");
	}
}

Flight TravellerScheduleReader::parseFlight(boost::property_tree::ptree& node) const {
	int source_sim = node.get<int>("source_sim");
	if (source_sim < 0) {
		throw invalid_argument("In TravellerScheduleReader: Unexpected simulator index.");
	}
	
	int destination_sim = node.get<int>("destination_sim");
	if (destination_sim < 0) {
		throw invalid_argument("In TravellerScheduleReader: Unexpected simulator index.");
	}
	
	int amount = node.get<int>("amount");
	if (amount < 0) {
		throw invalid_argument("In TravellerScheduleReader: Invalid amount of travellers.");
	}
	
	int duration = node.get<int>("duration");
	if (duration < 0) {
		throw invalid_argument("In TravellerScheduleReader: Invalid duration.");
	}
	
	int day_of_the_week = node.get<int>("day_of_the_week");
	if (day_of_the_week < 0 || day_of_the_week > 6) {
		throw invalid_argument("In TravellerScheduleReader: Invalid day of the week.");
	}

	return Flight(uint(source_sim), uint(destination_sim), uint(amount), uint(duration), uint(day_of_the_week));
}