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
#include "core/Infector.h"

#include <spdlog/spdlog.h>
#include "RngHandler.h"

namespace stride {

using namespace std;

/**
 * Primary R0_POLICY: do nothing i.e. track all cases.
 */
template<bool track_index_case = false>
class R0_POLICY {
public:
	static void Execute(Person* p) {}
};

/**
 * Specialized R0_POLICY: track only the index case.
 */
template<>
class R0_POLICY<true> {
public:
	static void Execute(Person* p) { p->getHealth().stopInfection(); }
};

/**
 * Primary LOG_POLICY policy, implements LogMode::None.
 */
template<LogMode log_level = LogMode::None>
class LOG_POLICY {
public:
	static void Execute(shared_ptr<spdlog::logger> logger, Person* p1, Person* p2,
						ClusterType cluster_type, shared_ptr<const Calendar> environ) {}
};

/**
 * Specialized LOG_POLICY policy LogMode::Transmissions.
 */
template<>
class LOG_POLICY<LogMode::Transmissions> {
public:
	static void Execute(shared_ptr<spdlog::logger> logger, Person* p1, Person* p2,
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
	static void Execute(shared_ptr<spdlog::logger> logger, Person* p1, Person* p2,
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
template<LogMode log_level, bool track_index_case>
void Infector<log_level, track_index_case>::execute(
		Cluster& cluster, DiseaseProfile disease_profile,
		RngHandler& contact_handler, shared_ptr<const Calendar> calendar) {
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
							if (contact_handler.hasTransmission(contact_rate,
																transmission_rate)) {
								LOG_POLICY<log_level>::Execute(logger, p1, p2, c_type, calendar);
								p2->getHealth().startInfection();
								R0_POLICY<track_index_case>::Execute(p2);
							}
						}
					}
				}
			}
		}
	}
}


//--------------------------------------------------------------------------
// Definition of partial specialization for LogMode::Contacts.
//--------------------------------------------------------------------------
template<bool track_index_case>
void Infector<LogMode::Contacts, track_index_case>::execute(
		Cluster& cluster, DiseaseProfile disease_profile,
		RngHandler& contact_handler, shared_ptr<const Calendar> calendar) {
	cluster.updateMemberPresence();

	// set up some stuff
	auto logger = spdlog::get("contact_logger");
	const auto c_type = cluster.m_cluster_type;
	const auto& c_members = cluster.m_members;
	//const auto c_size      = cluster.getSize();

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
						// TODO ContactHandler doesn't have a separate transmission function anymore to
						// check for transmission when contact has already been checked.
						// check for transmission
						/*bool transmission = contact_handler->transmission(age1, p2->getAge());
						unsigned int infecter = 0;
						if (transmission) {
								if (p1->IsInfectious() && p2->IsSusceptible()) {
										infecter = 1;
										p2->StartInfection();
										R0_POLICY<track_index_case>::Execute(p2);
								}
								else if (p2->isInfectious() && p1->isSusceptible()) {
										infecter = 2;
										p1->startInfection();
										R0_POLICY<track_index_case>::execute(p1);
								}
						}*/
						LOG_POLICY<LogMode::Contacts>::Execute(logger, p1, p2, c_type, calendar);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
// All explicit instantiations.
//--------------------------------------------------------------------------
template
class Infector<LogMode::None, false>;

template
class Infector<LogMode::None, true>;

template
class Infector<LogMode::Transmissions, false>;

template
class Infector<LogMode::Transmissions, true>;

template
class Infector<LogMode::Contacts, false>;

template
class Infector<LogMode::Contacts, true>;

}
