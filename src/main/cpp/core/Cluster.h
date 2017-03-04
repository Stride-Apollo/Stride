#pragma once
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

#include "core/ClusterType.h"
#include "core/ContactProfile.h"
#include "core/LogMode.h"
#include "pop/Person.h"

#include <array>
#include <cstddef>
#include <vector>
//#include <memory>

namespace stride {

class RngHandler;

class Calendar;

/**
 * Represents a location for social contacts, an group of people.
 */
class Cluster {
public:
	/// Constructor
	Cluster(std::size_t cluster_id, ClusterType cluster_type);

	/// Constructor
	//Cluster(const Cluster& rhs);

	/// Add the given Person to the Cluster.
	void addPerson(Person* p);

	/// Return number of persons in this cluster.
	std::size_t getSize() const { return m_members.size(); }

	/// Return the type of this cluster.
	ClusterType getClusterType() const { return m_cluster_type; }

	/// Get basic contact rate in this cluster.
	double getContactRate(const Person* p) const {
		return g_profiles.at(toSizeType(m_cluster_type))[effectiveAge(p->getAge())] / m_members.size();;
	}

public:
	/// Add contact profile.
	static void addContactProfile(ClusterType cluster_type, const ContactProfile& profile);

private:
	/// Sort members w.r.t. health status (order: exposed/infected/recovered, susceptible, immune).
	std::tuple<bool, size_t> sortMembers();

	/// Infector calculates contacts and transmissions.
	template<LogMode log_level, bool track_index_case>
	friend
	class Infector;

	/// Calculate which members are present in the cluster on the current day.
	void updateMemberPresence();

private:
	std::size_t m_cluster_id;     ///< The ID of the Cluster (for logging purposes).
	ClusterType m_cluster_type;   ///< The type of the Cluster (for logging purposes).
	std::size_t m_index_immune;   ///< Index of the first immune member in the Cluster.
	std::vector<std::pair<Person*, bool>> m_members;        ///< Container with pointers to Cluster members.
	const ContactProfile& m_profile;
private:
	static std::array<ContactProfile, numOfClusterTypes()> g_profiles;
};

}

