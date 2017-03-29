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

	void sortWorkplaces();

	void makeWork();

	void makeCommunities();

	template<typename T>
	vector<uint> getClusters(GeoCoordinate coord, double radius, const vector<T> clusters) const {
		vector<uint> result;
		const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
		for (uint i = 0; i < clusters.size(); i++) {
			if (calc.getDistance(coord, clusters.at(i).m_coord) <= radius) {
				result.push_back(i);
			}
		}
		return result;
	}

	void assignToSchools();

	void assignToUniversities();

	void assignCommutingStudent(SimplePerson& person);

	void assignCloseStudent(SimplePerson& person, double start_radius);

	void assignToWork();

	void assignCommutingEmployee(SimplePerson& person);

	void assignCloseEmployee(SimplePerson& person, double start_radius);

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
	vector<vector<SimpleCluster> > m_optional_schools;	/// One univ is a vector of clusters, ordering is the same as the cities they belong to (using modulo of course)

	/// TODO refactor this, it should be this structure from the beginning (see m_mandatory_schools)
	vector<vector<SimpleCluster> > m_mandatory_schools_clusters;

	uint m_next_id;

	/// Data for visualisation
	// TODO: population density still missing, not sure what to expect
	map<uint, uint> m_age_distribution;
	map<uint, uint> m_household_size;
	map<uint, uint> m_work_size;
};

}
}

