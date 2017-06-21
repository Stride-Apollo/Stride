#pragma once

#include <vector>
#include <string>
#include <map>

#include "AsyncSimulator.h"
#include "LocalSimulatorAdapter.h"
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
		// TODO: also give the simulators a name
		
		// Make the communication map
		uint id = 0;
		map<uint, AsyncSimulator*> communication_map;

		for (auto sim: m_sims) {
			sim->setId(id);
			communication_map[id] = sim;
			++id;
		}

		for (auto sim: m_sims) {
			// Note: this map contains all simulators, including "sim", this should not be a problem though
			// This allows "low-budget travels" within one region e.g. flemish people visiting the ardennes

			// TODO remove this troll
			auto some_sim = dynamic_cast<LocalSimulatorAdapter*>(sim);
			some_sim->setCommunicationMap(communication_map);
		}
	}

	void timeStep();

private:
	vector<AsyncSimulator*> m_sims;
	Schedule m_traveller_schedule;
};

}
