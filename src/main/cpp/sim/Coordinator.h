#pragma once

#include <vector>
#include <string>

#include "AsyncSimulator.h"
#include "util/TravellerScheduleReader.h"

namespace stride {

using namespace std;
using namespace util;

class Coordinator {
public:
	Coordinator(const map<string, shared_ptr<AsyncSimulator>>& sims, const string& filename)
			: m_sims(sims) {
		if (filename != "") {
			// TODO Fix traveller schedule
			//m_traveller_schedule = TravellerScheduleReader().readSchedule(filename);
		}
	}

	// TODO: Make this return a list of infected counts?
	void timeStep();

	// TODO: Calendars are saved for every Simulator *and* the Coordinator
	/// Return the calendar of this Coordinator
	const Calendar& getCalendar() const { return *m_calendar; }

private:
	map<string, shared_ptr<AsyncSimulator>> m_sims;
	Schedule m_traveller_schedule;
	std::shared_ptr<Calendar> m_calendar;
};

}
