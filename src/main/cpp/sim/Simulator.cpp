/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of the Simulator class.
 */

#include "Simulator.h"

#include "calendar/Calendar.h"
#include "calendar/DaysOffStandard.h"
#include "core/Infector.h"
#include "pop/Population.h"
#include "core/Cluster.h"
#include "util/unipar.h"

#include <random>

namespace stride {

using namespace std;
using namespace boost::property_tree;
using namespace stride::util;

Simulator::Simulator()
        : m_config_pt(), m_num_threads(1U), m_log_level(LogMode::Null),
		  m_information_policy(InformationPolicy::Null), m_population(nullptr),
          m_disease_profile(), m_track_index_case(false) {
	m_parallel = Parallel().withFunc<RandomRef>(std::function<RandomRef()>([&](){
		#if UNIPAR_IMPL == UNIPAR_DUMMY
			//std::cout << "Dummy rng?\n";
			return m_rng.get();
		#else
			// Use the rng to initialize a seed
			return make_unique<Random>(m_rng->operator()());
			//if (p.get() == nullptr) {
			//	std::cout << "We're passing on null rng?\n";
			//} else {
			//	std::cout << "ONE MORE RNG!\n";
			//}
			//return p;
		#endif
	}));
	//m_parallel.getResourceManager().setFunc();
}

const shared_ptr<const Population> Simulator::getPopulation() const {
	return m_population;
}

void Simulator::setTrackIndexCase(bool track_index_case) {
	m_track_index_case = track_index_case;
}

template<LogMode log_level, bool track_index_case, InformationPolicy information_policy>
void Simulator::updateClusters() {
	// Slight hack (thanks to http://stackoverflow.com/q/31724863/2678118#comment51385875_31724863)
	// but saves us a lot of typing without resorting to macro's.
	for (auto clusters: {&m_households, &m_school_clusters, &m_work_clusters,
						 &m_primary_community, &m_secondary_community}) {
		m_parallel.for_(0, clusters->size(), [&](RandomRef& rng, size_t i) {
			if (rng) {
				Infector<log_level, track_index_case¸ information_policy>::execute(
						(*clusters)[i], m_disease_profile, *rng, m_calendar);
				//std::cout << "rng is not null" << endl;
			} else {
				std::cout << "rng is null" << endl;
				_exit(1);
			}
		});
	}
}

void Simulator::timeStep() {
	shared_ptr<DaysOffInterface> days_off {nullptr};

	// Logic where you compute (on the basis of input/config for initial day
	// or on the basis of number of sick persons, duration of epidemic etc)
	// what kind of DaysOff scheme you apply. If we want to make this cluster
	// dependent then the days_off object has to be passed into the update function.
	days_off = make_shared<DaysOffStandard>(m_calendar);
	const bool is_work_off {days_off->isWorkOff()};
	const bool is_school_off {days_off->isSchoolOff()};

	double fraction_infected = m_population->GetFractionInfected();

	for (auto& p : *m_population) {
			p.update(is_work_off, is_school_off, fraction_infected);
	}

	if (m_track_index_case) {
		switch (m_information_policy) {
		case InformationPolicy::Global:
			switch (m_log_level) {
			case LogMode::Contacts:
				updateClusters<LogMode::Contacts, true, InformationPolicy::Global>(); break;
			case LogMode::Transmissions:
				updateClusters<LogMode::Transmissions, true, InformationPolicy::Global>(); break;
			case LogMode::None:
				updateClusters<LogMode::None, true, InformationPolicy::Global>(); break;
			default:
				throw runtime_error(std::string(__func__) + " Log mode screwed up!");
		} break;
		case InformationPolicy::Local:
			switch (m_log_level) {
			case LogMode::Contacts:
				updateClusters<LogMode::Contacts, true, InformationPolicy::Local>(); break;
			case LogMode::Transmissions:
				updateClusters<LogMode::Transmissions, true, InformationPolicy::Local>(); break;
			case LogMode::None:
				updateClusters<LogMode::None, true, InformationPolicy::Local>(); break;
			default:
				throw runtime_error(std::string(__func__) + " Log mode screwed up!");
			} break;
		default:
			throw runtime_error(std::string(__func__) + " Information policy screwed up!");
		}
	} else {
		switch (m_information_policy) {
		case InformationPolicy::Global:
			switch (m_log_level) {
			case LogMode::Contacts:
				updateClusters<LogMode::Contacts, false, InformationPolicy::Global>(); break;
			case LogMode::Transmissions:
				updateClusters<LogMode::Transmissions, false, InformationPolicy::Global>(); break;
			case LogMode::None:
				updateClusters<LogMode::None, false, InformationPolicy::Global>(); break;
			default:
				throw runtime_error(std::string(__func__) + " Log mode screwed up!");
			}
			break;
		case InformationPolicy::Local:
			switch (m_log_level) {
			case LogMode::Contacts:
				updateClusters<LogMode::Contacts, false, InformationPolicy::Local>(); break;
			case LogMode::Transmissions:
				updateClusters<LogMode::Transmissions, false, InformationPolicy::Local>(); break;
			case LogMode::None:
				updateClusters<LogMode::None, false, InformationPolicy::Local>(); break;
			default:
				throw runtime_error(std::string(__func__) + " Log mode screwed up!");
			}
			break;
			default:
				throw runtime_error(std::string(__func__) + " Information policy screwed up!");
		}
	}

	notify(*this);
	m_calendar->advanceDay();
}

const vector<Cluster>& Simulator::getClusters(ClusterType cluster_type) const {
	switch (cluster_type) {
		case ClusterType::Household:
			return m_households;
		case ClusterType::School:
			return m_school_clusters;
		case ClusterType::Work:
			return m_work_clusters;
		case ClusterType::PrimaryCommunity:
			return m_primary_community;
		case ClusterType::SecondaryCommunity:
			return m_secondary_community;
		default:
			throw runtime_error(string(__func__) + "> Should not reach default.");
	}
}

vector<string> Simulator::getRngStates() const {
	vector<string> states;
	stringstream ss;
	ss << *m_rng;
	states.push_back(ss.str());
	return states;
}

void Simulator::setRngStates(vector<string> states) {
	m_rng->setState(states.at(0));
}

}
