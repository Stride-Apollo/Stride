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
#include "AsyncSimulator.h"

#include "calendar/Calendar.h"
#include "calendar/DaysOffStandard.h"
#include "core/Infector.h"
#include "pop/Population.h"
#include "core/Cluster.h"
#include "util/unipar.h"
#include "util/GeoCoordCalculator.h"
#include "util/etc.h"

#include <random>
#include <algorithm>
#include <mutex>

namespace stride {

using namespace std;
using namespace boost::property_tree;
using namespace stride::util;

Simulator::Simulator()
        : m_config_pt(), m_num_threads(1U), m_log_level(LogMode::Null),
		  m_information_policy(InformationPolicy::Null), m_population(nullptr),
          m_disease_profile(), m_track_index_case(false), m_next_id(0), m_next_hh_id(0) {
	m_parallel.resources().setFunc([&](){
		#if UNIPAR_IMPL == UNIPAR_DUMMY
			return m_rng.get();
		#else
			std::random_device rd;
			return make_unique<Random>(rd());
		#endif
	});
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
				Infector<log_level, track_index_case, information_policy>::execute(
						(*clusters)[i], m_disease_profile, *rng, m_calendar);
		});
	}
}

void Simulator::timeStep() {
	// Advance the "calendar" of the districts (for the sphere of influence)
	for (auto& district: m_districts) {
		district.advanceInfluencesRecords();
	}

	shared_ptr<DaysOffInterface> days_off {nullptr};

	// Logic where you compute (on the basis of input/config for initial day
	// or on the basis of number of sick persons, duration of epidemic etc)
	// what kind of DaysOff scheme you apply. If we want to make this cluster
	// dependent then the days_off object has to be passed into the update function.
	days_off = make_shared<DaysOffStandard>(m_calendar);
	const bool is_work_off {days_off->isWorkOff()};
	const bool is_school_off {days_off->isSchoolOff()};

	double fraction_infected = m_population->getFractionInfected();

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

	m_calendar->advanceDay();
	m_planner.nextDay();
	this->notify(*this);
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

uint Simulator::chooseCluster(const GeoCoordinate& coordinate, const vector<Cluster>& clusters, double influence) {
	// TODO extend with sphere of influence
	vector<uint> available_clusters;
	const auto& calc = GeoCoordCalculator::getInstance();

	for (uint i = 1; i < clusters.size(); ++i) {

		const auto& cluster = clusters.at(i);

		if (calc.getDistance(coordinate, cluster.getLocation()) <= influence) {
			available_clusters.push_back(i);
		}
	}

	if (available_clusters.size() != 0) {
		uint chosen_index = m_rng->operator() (available_clusters.size());
		return available_clusters[chosen_index];

	} else {
		return clusters.size();
	}
}

bool Simulator::hostForeignTravellers(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) {
	GeoCoordinate facility_location;
	double influence = 0.0;
	bool found_airport = false;

	for (auto& district: this->m_districts) {
		if (district.getName() == destination_district && district.hasFacility(destination_facility)) {
			found_airport = true;
			facility_location = district.getLocation();
			influence = district.getFacilityInfluence(destination_facility);
			district.visitFacility(destination_facility, travellers.size());
			break;
		}
	}

	if (!found_airport) {
		return false;
	}

	// So that the addresses don't break, reserve the space needed in the vector
	this->m_population.get()->m_visitors.getDay(days);
	this->m_population.get()->m_visitors.getModifiableDay(days)->reserve(travellers.size());

	for (const Simulator::TravellerType& traveller: travellers) {
		// Choose the clusters the traveller will reside in
		uint work_index = this->chooseCluster(facility_location, this->m_work_clusters, influence);
		uint prim_comm_index = this->chooseCluster(facility_location, this->m_primary_community, influence);
		uint sec_comm_index = this->chooseCluster(facility_location, this->m_secondary_community, influence);

		if (work_index == this->m_work_clusters.size()
			|| prim_comm_index == this->m_primary_community.size()
			|| sec_comm_index == this->m_secondary_community.size()) {

			throw runtime_error("Failed to find cluster for traveller");
		}

		// Make the person
		uint start_infectiousness = traveller.getHomePerson().getHealth().getStartInfectiousness();
		uint start_symptomatic = traveller.getHomePerson().getHealth().getStartSymptomatic();

		// Note: the "ID" given to the constructor of a person is actually an index!
		Simulator::PersonType new_person = Simulator::PersonType(m_next_id, traveller.getHomePerson().getAge(), m_next_hh_id, 0,
																	work_index, prim_comm_index, sec_comm_index,
																	start_infectiousness, start_symptomatic,
																	traveller.getHomePerson().getHealth().getEndInfectiousness() - start_infectiousness,
																	traveller.getHomePerson().getHealth().getEndSymptomatic() - start_symptomatic);
		new_person.getHealth() = traveller.getHomePerson().getHealth();

		// Add the person to the planner
		this->m_population->m_visitors.add(days, new_person);

		// Note: the ID of a non-traveller is always the same as his index in m_population->m_original
		Simulator::TravellerType new_traveller = Simulator::TravellerType(traveller.getHomePerson(),
																			this->m_population->m_visitors.getModifiableDay(days)->back().get(),
																			traveller.getHomeSimulatorId(),
																			traveller.getDestinationSimulatorId(),
																			traveller.getHomePerson().getId());

		new_traveller.getNewPerson()->setOnVacation(false);
		m_planner.add(days, new_traveller);

		// Get a pointer to the person you just made
		Simulator::PersonType* person = this->m_population->m_visitors.getModifiableDay(days)->back().get();

		// Add the person to the clusters
		this->m_work_clusters.at(work_index).addPerson(person);
		this->m_primary_community.at(prim_comm_index).addPerson(person);
		this->m_secondary_community.at(sec_comm_index).addPerson(person);

		++m_next_id;
		++m_next_hh_id;
	}

	return true;
}

bool Simulator::welcomeHomeTravellers(const vector<uint>& travellers_indices, const vector<Health>& health_status) {
	auto& original_population = m_population->m_original;
	for (uint i = 0; i < travellers_indices.size(); ++i) {
		original_population.at(travellers_indices.at(i)).setOnVacation(false);
		original_population.at(travellers_indices.at(i)).getHealth() = health_status.at(i);
	}

	return true;
}

void Simulator::returnForeignTravellers() {

	// Get the people that return home today (according to the planner in the population of this simulator)
	SimplePlanner<Simulator::TravellerType>::Block* returning_people = m_planner.getModifiableDay(0);

	uint max_sim_id = 0;
	for (auto it = returning_people->begin(); it != returning_people->end(); ++it) {
		max_sim_id = std::max(uint(max_sim_id), uint((**it).getHomeSimulatorId()));
	}
	++max_sim_id;

	vector<pair<vector<uint>, vector<Health> > > result (max_sim_id, pair<vector<uint>, vector<Health> >());

	for (auto it = returning_people->begin(); it != returning_people->end(); ++it) {
		auto& traveller = **it;

		Simulator::PersonType* returning_person = traveller.getNewPerson();

		// Get the clusters
		auto work_index = returning_person->getClusterId(ClusterType::Work);
		auto prim_comm_index = returning_person->getClusterId(ClusterType::PrimaryCommunity);
		auto sec_comm_index = returning_person->getClusterId(ClusterType::SecondaryCommunity);

		// Remove him from the clusters
		m_work_clusters.at(work_index).removePerson(returning_person->getId());
		m_primary_community.at(prim_comm_index).removePerson(returning_person->getId());
		m_secondary_community.at(sec_comm_index).removePerson(returning_person->getId());

		uint destination_sim_id = traveller.getHomeSimulatorId();

		// Make the output
		result.at(destination_sim_id).first.push_back(traveller.getHomePerson().getId());
		result.at(destination_sim_id).second.push_back(traveller.getNewPerson()->getHealth());
	}

	m_planner.nextDay();
	m_population->m_visitors.nextDay();

	// Give the data to the senders
	for (int i = 0; i < result.size(); ++i) {
		if (result.at(i).second.size() != 0) {
			m_async_sim->returnForeignTravellers(result.at(i), i);
		}
	}
}

void Simulator::sendNewTravellers(uint amount, uint days, uint destination_sim_id, string destination_district, string destination_facility) {
	list<Simulator::PersonType*> working_people;

	// Get the working people
	Population& population = *(m_population.get());
	for (uint i = 0; i < population.m_original.size(); ++i) {
		Simulator::PersonType& person = population.m_original.at(i);
		if (person.getClusterId(ClusterType::Work) != 0 && !person.isOnVacation()) {
			working_people.push_back(&person);
		}
	}

	if (amount > working_people.size()) {
		// TODO throw exception
		cout << "Warning, more people to send than actual people in region.\n";
	}

	vector<Simulator::TravellerType> chosen_people;
	//chosen_people.reserve(amount);

	while (chosen_people.size() != amount) {
		// Randomly generate an index in working_people
		unsigned int index = m_rng->operator() (working_people.size());

		// Get the person to be sent
		Simulator::PersonType* person = *(next(working_people.begin(), index));
		person->setOnVacation(true);

		// Make the traveller and make sure he can't be sent twice
		Simulator::TravellerType new_traveller = Simulator::TravellerType(*person, nullptr, m_id, destination_sim_id, person->getId());
		chosen_people.push_back(new_traveller);
		working_people.erase(next(working_people.begin(), index));

	}

	m_async_sim->sendNewTravellers(chosen_people, days, destination_sim_id, destination_district, destination_facility);
}

}
