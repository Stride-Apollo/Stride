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
	template <typename Iter>
	Coordinator(const Iter& sims, const string& filename = "") : m_sims(sims.begin(), sims.end()) {
		if (filename != "") {
			TravellerScheduleReader reader;
			m_traveller_schedule = reader.readSchedule(filename);
		}
		initializeSimulators();
	}

	Coordinator(initializer_list<AsyncSimulator*> sims)
		: m_sims(sims.begin(), sims.end()) {
			initializeSimulators();}

	void initializeSimulators() {
		string id = "Simulator";
		for (int i = 0; i<m_sims.size(); i++){
			m_sims.at(i)->setId(id+to_string(i));
		}
	}

	void timeStep();

private:
	vector<AsyncSimulator*> m_sims;
	Schedule m_traveller_schedule;
};

}
