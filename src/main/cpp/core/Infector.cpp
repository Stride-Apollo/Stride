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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of Infector algorithms.
 */

#include "calendar/Calendar.h"
#include "core/Cluster.h"
#include "core/Health.h"
#include "core/Infector.h"
#include "core/LogMode.h"
#include "pop/Person.h"
#include "util/Random.h"

#include <spdlog/spdlog.h>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace stride {

using namespace std;
using namespace util;

/**
 * Primary R0_POLICY: do nothing i.e. track all cases.
 */
template<bool track_index_case = false>
class R0_POLICY {
public:
	static void execute(Simulator::PersonType* p) {}
};

/**
 * Specialized R0_POLICY: track only the index case.
 */
template<>
class R0_POLICY<true> {
public:
	static void execute(Simulator::PersonType* p) { p->getHealth().stopInfection(); }
};

/**
 * Primary LOG_POLICY policy, implements LogMode::None.
 */
template<LogMode log_level = LogMode::None>
class LOG_POLICY {
public:
	static void execute(shared_ptr<spdlog::logger> logger, Simulator::PersonType* p1, Simulator::PersonType* p2,
						ClusterType cluster_type, shared_ptr<const Calendar> environ) {}
};

/**
 * Specialized LOG_POLICY policy LogMode::Transmissions.
 */
template<>
class LOG_POLICY<LogMode::Transmissions> {
public:
	static void execute(shared_ptr<spdlog::logger> logger, Simulator::PersonType* p1, Simulator::PersonType* p2,
						ClusterType cluster_type, shared_ptr<const Calendar> environ) {
		logger->info("[TRAN] {} {} {} {}",
					 p1->getId(), p2->getId(), toString(cluster_type), environ->getSimulationDay());
	}
};

/**
 * Specialized LOG_POLICY policy LogMode::Contacts.
 */
template<>
class LOG_POLICY<LogMode::Contacts> {
public:
	static void execute(shared_ptr<spdlog::logger> logger, Simulator::PersonType* p1, Simulator::PersonType* p2,
						ClusterType cluster_type, shared_ptr<const Calendar> calendar) {
		unsigned int home = (cluster_type == ClusterType::Household);
		unsigned int work = (cluster_type == ClusterType::Work);
		unsigned int school = (cluster_type == ClusterType::School);
		unsigned int primary_community = (cluster_type == ClusterType::PrimaryCommunity);
		unsigned int secundary_community = (cluster_type == ClusterType::SecondaryCommunity);

		logger->info("[CONT] {} {} {} {} {} {} {} {} {}",
					 p1->getId(), p1->getAge(), p2->getAge(), home, school, work, primary_community,
					 secundary_community,
					 calendar->getSimulationDay());
	}
};

//--------------------------------------------------------------------------
// Definition for primary template covers the situation for
// LogMode::None & LogMode::Transmissions, both with
// track_index_case false and true..
//--------------------------------------------------------------------------
template<LogMode log_level, bool track_index_case, InformationPolicy information_policy>
void Infector<log_level, track_index_case, information_policy>::execute(
		Cluster& cluster, DiseaseProfile disease_profile,
		util::Random& contact_handler, shared_ptr<const Calendar> calendar) {
	// check if the cluster has infected members and sort
	bool infectious_cases;
	size_t num_cases;
	tie(infectious_cases, num_cases) = cluster.sortMembers();

	if (infectious_cases) {
		cluster.updateMemberPresence();

		// Set up some stuff
		auto logger = spdlog::get("contact_logger");
		const auto c_type = cluster.m_cluster_type;
		const auto c_immune = cluster.m_index_immune;
		const auto& c_members = cluster.m_members;
		const auto transmission_rate = disease_profile.getTransmissionRate();

		// Match infectious in first part with susceptible in second part, skip last part (immune)
		for (size_t i_infected = 0; i_infected < num_cases; i_infected++) {
			// check if member is present today
			if (c_members[i_infected].second) {
				const auto p1 = c_members[i_infected].first;
				if (p1->getHealth().isInfectious()) {
					const double contact_rate = cluster.getContactRate(p1);
					for (size_t i_contact = num_cases; i_contact < c_immune; i_contact++) {
						// check if member is present today
						if (c_members[i_contact].second) {
							auto p2 = c_members[i_contact].first;
							if (contact_handler.hasContactAndTransmission(contact_rate, transmission_rate)) {
								LOG_POLICY<log_level>::execute(logger, p1, p2, c_type, calendar);
								p2->getHealth().startInfection();
								R0_POLICY<track_index_case>::execute(p2);
							}
						}
					}
				}
			}
		}
	}
}


//--------------------------------------------------------------------------
// Definition of partial specialization for InformationPolicy::Local.
//--------------------------------------------------------------------------
template<LogMode log_level, bool track_index_case>
void Infector<log_level, track_index_case, InformationPolicy::Local>::execute(
        Cluster& cluster, DiseaseProfile disease_profile,
        util::Random& contact_handler, shared_ptr<const Calendar> calendar) {
	cluster.updateMemberPresence();

	// set up some stuff
	auto logger            = spdlog::get("contact_logger");
	const auto c_type      = cluster.m_cluster_type;
	const auto& c_members  = cluster.m_members;
	const auto transmission_rate = disease_profile.getTransmissionRate();

	// check all contacts
	for (size_t i_person1 = 0; i_person1 < cluster.m_members.size(); i_person1++) {
		// check if member participates in the social contact survey && member is present today
		if (c_members[i_person1].second) {
			auto p1 = c_members[i_person1].first;
			const double contact_rate = cluster.getContactRate(p1);
			for (size_t i_person2 = 0; i_person2 < c_members.size(); i_person2++) {
				// check if member is present today
				if ((i_person1 != i_person2) && c_members[i_person2].second) {
					auto p2 = c_members[i_person2].first;
					// check for contact
					if (contact_handler.hasContact(contact_rate)) {
						// let contacts influence each other's beliefs
						p1->update(p2);
						p2->update(p1);

						bool transmission = contact_handler.hasTransmission(transmission_rate);
						if (transmission) {
							if (p1->getHealth().isInfectious() && p2->getHealth().isSusceptible()) {
								p2->getHealth().startInfection();
								R0_POLICY<track_index_case>::execute(p2);
							} else if (p2->getHealth().isInfectious() && p1->getHealth().isSusceptible()) {
								p1->getHealth().startInfection();
								R0_POLICY<track_index_case>::execute(p1);
							}
						}

						LOG_POLICY<log_level>::execute(logger, p1, p2, c_type, calendar);
					}
				}
			}
		}
	}
}


// TODO find a solution for this AWFUL case of code duplication
//-------------------------------------------------------------------------------------------
// Definition of partial specialization for InformationPolicy::Global and LogMode::Contacts.
//-------------------------------------------------------------------------------------------
template<bool track_index_case>
void Infector<LogMode::Contacts, track_index_case, InformationPolicy::Global>::execute(
		Cluster& cluster, DiseaseProfile disease_profile,
		util::Random& contact_handler, shared_ptr<const Calendar> calendar) {
	cluster.updateMemberPresence();

	// set up some stuff
	auto logger            = spdlog::get("contact_logger");
	const auto c_type      = cluster.m_cluster_type;
	const auto& c_members  = cluster.m_members;
	const auto transmission_rate = disease_profile.getTransmissionRate();

	// check all contacts
	for (size_t i_person1 = 0; i_person1 < cluster.m_members.size(); i_person1++) {
		// check if member participates in the social contact survey && member is present today
		if (c_members[i_person1].second && c_members[i_person1].first->isParticipatingInSurvey()) {
			auto p1 = c_members[i_person1].first;
			const double contact_rate = cluster.getContactRate(p1);
			for (size_t i_person2 = 0; i_person2 < c_members.size(); i_person2++) {
				// check if member is present today
				if ((i_person1 != i_person2) && c_members[i_person2].second) {
					auto p2 = c_members[i_person2].first;
					// check for contact
					if (contact_handler.hasContact(contact_rate)) {
						bool transmission = contact_handler.hasTransmission(transmission_rate);
						if (transmission) {
							if (p1->getHealth().isInfectious() && p2->getHealth().isSusceptible()) {
								p2->getHealth().startInfection();
								R0_POLICY<track_index_case>::execute(p2);
							} else if (p2->getHealth().isInfectious() && p1->getHealth().isSusceptible()) {
								p1->getHealth().startInfection();
								R0_POLICY<track_index_case>::execute(p1);
							}
						}

						LOG_POLICY<LogMode::Contacts>::execute(logger, p1, p2, c_type, calendar);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
// All explicit instantiations.
//--------------------------------------------------------------------------
template class Infector<LogMode::None, false, InformationPolicy::Global>;
template class Infector<LogMode::None, false, InformationPolicy::Local>;

template class Infector<LogMode::None, true, InformationPolicy::Global>;
template class Infector<LogMode::None, true, InformationPolicy::Local>;

template class Infector<LogMode::Transmissions, false, InformationPolicy::Global>;
template class Infector<LogMode::Transmissions, false, InformationPolicy::Local>;

template class Infector<LogMode::Transmissions, true, InformationPolicy::Global>;
template class Infector<LogMode::Transmissions, true, InformationPolicy::Local>;

template class Infector<LogMode::Contacts, false, InformationPolicy::Global>;
template class Infector<LogMode::Contacts, false, InformationPolicy::Local>;

template class Infector<LogMode::Contacts, true, InformationPolicy::Global>;
template class Infector<LogMode::Contacts, true, InformationPolicy::Local>;


}
