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

using uint = unsigned int;
using namespace std;
using namespace util;

using Cluster = vector<uint>;

/// Contains the indices of the people in Population::all
using SimpleFamily = vector<uint>;

struct Population {
	vector<SimplePerson> all;
	vector<SimpleFamily> families;

	/// TODO: refactor: Didn't use this yet, I don't think we need it (Sam)
	vector<Cluster> schools;
	vector<Cluster> workplaces;
	vector<Cluster> communities;
};

struct SimpleSchool {
	uint m_current_pupils = 0;
	uint m_id = 0;
};

std::ostream& operator<<(std::ostream& os, const Population& p);

class PopulationGenerator {
public:

	PopulationGenerator(const string& filename);

	Population generate();

private:
	void makeSchools(const map<uint, uint>& age_map, Population& pop);
	void makeWork(const map<uint, uint>& age_map, Population& pop);
	void makeCommunities(const map<uint, uint>& age_map, Population& pop);

	boost::property_tree::ptree m_props;
	uint m_total;
	map<uint, double> m_family_size_fractions;
	MappedAliasDistribution m_no_kids_family_size_dist;
	MappedAliasDistribution m_some_kids_family_size_dist;
	double m_family_size_avg = 0;
	double m_no_kids_family_size_avg = 0;
	MinMax m_family_size = MinMax(numeric_limits<uint>::max(),
								  numeric_limits<uint>::min());

	MinMax m_age_kids;
	MinMax m_age_parents;
	MinMax m_age_diff_kids;
	uint m_age_diff_parents_max = 0;
	uint m_age_diff_parents_kids_min = 0;
	uint m_age_no_kids_min = 0;
	uint m_cluster_id = 0;
};

}
}

