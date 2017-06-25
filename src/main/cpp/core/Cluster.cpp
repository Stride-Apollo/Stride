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
 * Header for the core Cluster class.
 */

#include "Cluster.h"

#include "Infector.h"
#include "calendar/Calendar.h"

#include <spdlog/spdlog.h>

namespace stride {

using namespace std;

std::array<ContactProfile, numOfClusterTypes()> Cluster::g_profiles;

Cluster::Cluster(std::size_t cluster_id, ClusterType cluster_type, GeoCoordinate coordinate)
		: m_cluster_id(cluster_id), m_cluster_type(cluster_type),
		  m_index_immune(0), m_profile(g_profiles.at(toSizeType(m_cluster_type))),
		  m_coordinate(coordinate) {
}

void Cluster::addContactProfile(ClusterType cluster_type, const ContactProfile& profile) {
	g_profiles.at(toSizeType(cluster_type)) = profile;
}


void Cluster::addPerson(Simulator::PersonType* p) {
	m_members.emplace_back(std::make_pair(p, true));
	m_index_immune++;
}

std::size_t Cluster::getInfectedCount() const {
	size_t num_cases = 0;
	for (auto& member : m_members) {
		auto health = member.first->getHealth();
		if (health.isInfected() || health.isRecovered())
			++num_cases;
	}
	return num_cases;
}

void Cluster::removePerson(unsigned int id) {
	for (unsigned int i_member = 0; i_member < m_members.size(); ++i_member) {
		if (m_members.at(i_member).first->getId() == id) {
			m_members.erase(m_members.begin() + i_member);
			m_index_immune = m_members.size() > 0 ? m_members.size() - 1 : 0;
			return;
		}
	}
}

std::size_t Cluster::getActiveClusterMembers() const {
	std::size_t total = 0;
	for (const auto& person: m_members) {
		if (!person.first->isOnVacation()) {
			++total;
		}
	}

	return total;
}

tuple<bool, size_t> Cluster::sortMembers() {
	bool infectious_cases = false;
	size_t num_cases = 0;

	for (size_t i_member = 0; i_member < m_index_immune; i_member++) {
		// if immune, move to back
		if (m_members[i_member].first->getHealth().isImmune()) {
			bool swapped = false;
			size_t new_place = m_index_immune - 1;
			m_index_immune--;
			while (!swapped && new_place > i_member) {
				if (m_members[new_place].first->getHealth().isImmune()) {
					m_index_immune--;
					new_place--;
				} else {
					swap(m_members[i_member], m_members[new_place]);
					swapped = true;
				}
			}
		}
			// else, if not susceptible, move to front
		else if (!m_members[i_member].first->getHealth().isSusceptible()) {
			if (!infectious_cases && m_members[i_member].first->getHealth().isInfectious()) {
				infectious_cases = true;
			}
			if (i_member > num_cases) {
				swap(m_members[i_member], m_members[num_cases]);
			}
			num_cases++;
		}
	}
	return make_tuple(infectious_cases, num_cases);
}

void Cluster::updateMemberPresence() {
	for (auto& member: m_members) {
		member.second = member.first->isInCluster(m_cluster_type);
	}
}

}
