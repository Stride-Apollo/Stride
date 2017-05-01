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
		: m_sims(sims.begin(), sims.end()) {initializeSimulators();}

	void initializeSimulators() {
		uint id = 0;
		for (auto sim: m_sims) {
			sim->setId(id);
			++id;
		}
	}

	void timeStep();

	vector<TravellerData> forceReturnTravellers();

	void forceSendTravellers(const vector<TravellerData>& traveller_data);

private:
	vector<AsyncSimulator*> m_sims;
	Schedule m_traveller_schedule;
};

}
