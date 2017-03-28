#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <random>
#include <cassert>
#include <exception>
#include <limits>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "util/AliasDistribution.h"

#include "utils.h"

namespace stride {
namespace popgen {

using namespace std;
using namespace util;

using uint = unsigned int;

class PopulationGenerator {
public:

	PopulationGenerator(const string& filename);

	void generate();

private:
	void makeRNG();

	void makeHouseholds();

	void makeCities();

	GeoCoordinate getCityMiddle() const;

	double getCityRadius(const GeoCoordinate& coord) const;
	/// Distance between the given coordinate and the furthest city

	double getCityPopulation() const;

	double getVillagePopulation() const;

	void makeVillages();

	void placeHouseholds();

	void placeClusters(uint size, uint min_age, uint max_age, double fraction, vector<SimpleCluster>& clusters);
	/// Spreads the clusters of people with these constraints over the cities and villages

	void makeSchools();

	void makeUniversities();

	void makeWork();

	void makeCommunities();

	void assignToSchools();

	void assignToUniversities();

	void assignToWork();

	void assignToCommunities();

	boost::property_tree::ptree m_props;
	RNGPicker m_rng;
	uint m_total;
	vector<SimplePerson> m_people;
	vector<SimpleHousehold> m_households;
	vector<SimpleCity> m_cities;
	vector<SimpleCluster> m_villages;
	vector<SimpleCluster> m_workplaces;
	vector<SimpleCluster> m_primary_communities;
	vector<SimpleCluster> m_secondary_communities;
	vector<SimpleCluster> m_mandatory_schools;
	vector<SimpleCluster> m_optional_schools;
	uint m_next_id;

	/// Data for visualisation
	// TODO: population density still missing, not sure what to expect
	map<uint, uint> m_age_distribution;
	map<uint, uint> m_household_size;
	map<uint, uint> m_work_size;
};

}
}

