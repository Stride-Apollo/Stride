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

	void makeVillages();

	void placeHouseholds();

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
	vector<SimpleCluster> m_clusters;
	vector<SimpleSchool> m_mandatory_schools;
	vector<SimpleSchool> m_optional_schools;
	uint m_next_id;

	/// Data for visualisation
	// TODO: population density still missing, not sure what to expect
	map<uint, uint> m_age_distribution;
	map<uint, uint> m_household_size;
	map<uint, uint> m_work_size;
};

}
}

