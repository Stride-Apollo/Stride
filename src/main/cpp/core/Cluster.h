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
#include "pop/PopulationBuilder.h"
#include "sim/Simulator.h"
#include "util/GeoCoordinate.h"
#include "checkpointing/Loader.h"

#include <array>
#include <cstddef>
#include <vector>
//#include <memory>

namespace stride {

using namespace util;

class RngHandler;
class Calendar;

/**
 * Represents a location for social contacts, an group of people.
 */
class Cluster {
public:
	/// Constructor
	Cluster(std::size_t cluster_id, ClusterType cluster_type, GeoCoordinate coordinate = GeoCoordinate(0, 0));

	/// Constructor
	//Cluster(const Cluster& rhs);

	/// Add the given Person to the Cluster.
	void addPerson(Simulator::PersonType* p);

	/// Remove the given Person from the Cluster.
	void removePerson(unsigned int id);

	/// Return number of persons in this cluster.
	std::size_t getSize() const { return m_members.size(); }

	/// Return number of persons in this cluster.
	std::size_t getActiveClusterMembers() const;

	/// Return the amount of infected people in this cluster.
	std::size_t getInfectedCount() const;

	/// Return the type of this cluster.
	ClusterType getClusterType() const { return m_cluster_type; }

	/// Return the geo coordinates (latitude-longitude) of the cluster
	GeoCoordinate getLocation() const {return m_coordinate;}

	/// Get basic contact rate in this cluster.
	double getContactRate(const Simulator::PersonType* p) const {
		return g_profiles.at(toSizeType(m_cluster_type))[effectiveAge(p->getAge())] / m_members.size();;
	}

	/// Get the ID of this cluster
	std::size_t getId() const {return m_cluster_id;}

	/// Get the members of this vector
	/// Rather for testing purposes
	const std::vector<std::pair<Simulator::PersonType*, bool>>& getMembers() const {return m_members;}

public:
	/// Add contact profile.
	static void addContactProfile(ClusterType cluster_type, const ContactProfile& profile);

private:
	/// Sort members w.r.t. health status (order: exposed/infected/recovered, susceptible, immune).
	std::tuple<bool, size_t> sortMembers();

	/// Infector calculates contacts and transmissions.
	template<LogMode log_level, bool track_index_case, typename local_information_policy>
	friend class Infector;

	/// Calculate which members are present in the cluster on the current day.
	void updateMemberPresence();

private:
	std::size_t m_cluster_id;     ///< The ID of the Cluster (for logging purposes).
	ClusterType m_cluster_type;   ///< The type of the Cluster (for logging purposes).
	std::size_t m_index_immune;   ///< Index of the first immune member in the Cluster.
	std::vector<std::pair<Simulator::PersonType*, bool>> m_members;  ///< Container with pointers to Cluster members.
	const ContactProfile& m_profile;
	const GeoCoordinate m_coordinate;	///< The location of the cluster
private:
	static std::array<ContactProfile, numOfClusterTypes()> g_profiles;
private:
	friend class Loader;
	friend class Saver;
};

}
