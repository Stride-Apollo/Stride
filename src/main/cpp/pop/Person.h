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
 * Header file for the Person class.
 */

#include "core/Health.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <behavior/behavior_policies/NoBehavior.h>
#include <behavior/belief_policies/NoBelief.h>

namespace stride {

class Calendar;
template<class PersonType> class Traveller;
enum class ClusterType;

class Saver;

/**
 * Store and handle person data.
 */
template <class BehaviorPolicy, class BeliefPolicy>
class Person {
friend class Saver;
friend class Loader;
public:
	/// Constructor: set the person data.
	Person(unsigned int id, double age, unsigned int household_id, unsigned int school_id,
		   unsigned int work_id, unsigned int primary_community_id, unsigned int secondary_community_id,
		   unsigned int start_infectiousness,
		   unsigned int start_symptomatic, unsigned int time_infectious, unsigned int time_symptomatic,
	       bool is_on_vacation = false)
			: m_id(id), m_age(age), m_gender('M'),
			  m_household_id(household_id), m_school_id(school_id),
			  m_work_id(work_id), m_primary_community_id(primary_community_id),
			  m_secondary_community_id(secondary_community_id),
			  m_at_household(true), m_at_school(true), m_at_work(true), m_at_primary_community(true),
			  m_at_secondary_community(true),
			  m_health(start_infectiousness, start_symptomatic, time_infectious, time_symptomatic),
			  m_is_participant(false), m_is_on_vacation(is_on_vacation) {}

	/// Is this person not equal to the given person?
	bool operator!=(const Person& p) const { return p.m_id != m_id; }

	/// Get the age.
	double getAge() const { return m_age; }

	/// Get cluster ID of cluster_type
	unsigned int getClusterId(ClusterType cluster_type) const;

	/// Return person's gender.
	char getGender() const { return m_gender; }

	/// Return person's health status.
	Health& getHealth() { return m_health; }

	/// Return person's health status.
	const Health& getHealth() const { return m_health; }

	/// Get the id.
	unsigned int getId() const { return m_id; }

	/// Check if a person is present today in a given cluster
	bool isInCluster(ClusterType c) const;

	/// Does this person participates in the social contact study?
	bool isParticipatingInSurvey() const { return m_is_participant; }

	/// Participate in social contact study and log person details
	void participateInSurvey() { m_is_participant = true; }

	/// Update the health status and presence in clusters.
	void update(bool is_work_off, bool is_school_off);

	bool isOnVacation() const { return m_is_on_vacation; }
	void setOnVacation(bool is_on_vacation) { m_is_on_vacation = is_on_vacation; }

	template<class PersonType> friend class Traveller;

private:
	unsigned int m_id;  ///< The id.
	double m_age;       ///< The age.
	char m_gender;      ///< The gender.

	unsigned int m_household_id;           ///< The household id.
	unsigned int m_school_id;              ///< The school cluster id
	unsigned int m_work_id;                ///< The work cluster id
	unsigned int m_primary_community_id;   ///< The primary community id
	unsigned int m_secondary_community_id; ///< The secondary community id

	bool m_at_household;           ///< Is person present at household today?
	bool m_at_school;              ///< Is person present at school today?
	bool m_at_work;                ///< Is person present at work today?
	bool m_at_primary_community;   ///< Is person present at primary_community today?
	bool m_at_secondary_community; ///< Is person present at secundary_community today?

	Health m_health;                           ///< Health info for this person.
	typename BeliefPolicy::Data m_belief_data; ///< Info w.r.t. this Person's health beliefs

	bool m_is_participant;  ///< Is participating in the social contact study
	bool m_is_on_vacation;  ///< Is currently on a vacation and should be included in calculations
	                        ///< Note: Population already filters these people out when iterating
};

/// Explicit instantiations in .cpp file
extern template class Person<NoBehavior, NoBelief>;

}

