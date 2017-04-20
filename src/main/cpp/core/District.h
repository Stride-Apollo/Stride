#pragma once

/**
 * @file
 * Header for the District class.
 */

#include "util/GeoCoordinate.h"

#include <string>
#include <unordered_set>

namespace stride {

using namespace util;
using namespace std;

/**
 * A district is either a city or a village (currently, there is no difference between city and village)
 */
class District {
public:
	/// Constructor
	District(string district_name, GeoCoordinate location = GeoCoordinate(0,0)): m_name(district_name), m_location(location) {}

	/// Add a transportation facility to this district
	void addFacility(string facility_name) {m_transportations_facilities.insert(facility_name);}

	/// If the facility is found in this district, return true
	bool hasFacility(string facility_name) {return m_transportations_facilities.find(facility_name) != m_transportations_facilities.end();}

	/// Equals operator for districts, two districts are equal if their name is the same
	bool operator==(const District& other_district) {return other_district.m_name == m_name;}

	/// Return the name of the district
	string getName() const {return m_name;}

	GeoCoordinate getLocation() {return m_location;}

private:
	unordered_set<string> m_transportations_facilities;		///< The transportation facilities have a name (string)
															///< currently, only airports are considered (even though airports don't have any special properties at the moment)
	string m_name;											///< The name of the city/village
	const GeoCoordinate m_location;							///< The geographic location of the district
};

}

