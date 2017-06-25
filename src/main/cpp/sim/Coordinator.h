#pragma once

#include <vector>
#include <string>
#include <boost/property_tree/ptree.hpp>

#include "calendar/Calendar.h"
#include "AsyncSimulator.h"
#include "util/TravellerScheduleReader.h"

namespace stride {

using namespace std;
using namespace util;

class Coordinator {
public:
	Coordinator(const map<string, shared_ptr<AsyncSimulator>>& sims, const string& schedule,
				boost::property_tree::ptree& config)
			: m_sims(sims), m_calendar(config) {
		if (schedule != "") {
			// TODO Fix traveller schedule
			//m_traveller_schedule = TravellerScheduleReader().readSchedule(filename);
		}
	}

	// TODO: Make this return a list of infected counts?
	void timeStep();


private:
	Schedule m_traveller_schedule;
	map<string, shared_ptr<AsyncSimulator>> m_sims;
	// TODO: Calendars are saved for every Simulator *and* the Coordinator
	Calendar m_calendar;
};

}
