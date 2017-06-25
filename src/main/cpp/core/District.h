#pragma once

/**
 * @file
 * Header for the District class.
 */

#include "util/GeoCoordinate.h"
#include "core/Influence.h"

#include <string>
#include <utility>
#include <algorithm>
#include <vector>

namespace stride {

using namespace util;
using namespace std;

class ClusterSaver;

/**
 * A district is either a city or a village (currently, there is no difference between city and village)
 */
class District {
public:
	/// Constructor
	District(string district_name, uint influence_size, double influence_speed, double influence_minimum, GeoCoordinate location = GeoCoordinate(0,0)):
		m_name(district_name),
		m_location(location),
		m_influence_size(influence_size),
		m_influence_speed(influence_speed),
		m_influence_minimum(influence_minimum) {}

	/// Add a transportation facility to this district
	void addFacility(string facility_name) {
		if (! hasFacility(facility_name)) {
			m_transportations_facilities.push_back(make_pair(facility_name, Influence(m_influence_size, m_influence_speed, m_influence_minimum)));
		}
	}

	/// If the facility is found in this district, return true
	bool hasFacility(string facility_name) const {
		return getFacility(facility_name) != m_transportations_facilities.end();
	}

	double getFacilityInfluence(string facility_name) const {
		if (hasFacility(facility_name)) {
			auto it = getFacility(facility_name);
			return it->second.getInfluence();
		}

		return 0.0;
	}

	void visitFacility(string facility_name, uint amount) {
		if (hasFacility(facility_name)) {
			auto it = getFacility(facility_name);
			it->second.addToFront(amount);
		}
	}

	void advanceInfluencesRecords() {
		for (auto& facility: m_transportations_facilities) {
			facility.second.addRecord(0);
		}
	}

	/// Equals operator for districts, two districts are equal if their name is the same
	bool operator==(const District& other_district) {return other_district.m_name == m_name;}

	/// Return the name of the district
	string getName() const {return m_name;}

	GeoCoordinate getLocation() const {return m_location;}

private:
	vector<pair<string, Influence> > m_transportations_facilities;		///< The transportation facilities have a name (string) and a sphere of influence
	string m_name;						///< The name of the city/village
	const GeoCoordinate m_location;		///< The geographic location of the district
	const uint m_influence_size;		///< All Influences of facilities will get this size
	const uint m_influence_speed;		///< All Influences of facilities will get this speed
	const uint m_influence_minimum;		///< All Influences of facilities will get this minimum

	vector<pair<string, Influence> >::const_iterator getFacility(string facility_name) const {
		auto same_name = [&] (const pair<string, Influence>& facility) {return facility.first == facility_name;};
		return find_if(m_transportations_facilities.begin(), m_transportations_facilities.end(), same_name);
	}

	vector<pair<string, Influence> >::iterator getFacility(string facility_name) {
		auto same_name = [&] (const pair<string, Influence>& facility) {return facility.first == facility_name;};
		return find_if(m_transportations_facilities.begin(), m_transportations_facilities.end(), same_name);
	}

	friend class ClusterSaver;
};

}

