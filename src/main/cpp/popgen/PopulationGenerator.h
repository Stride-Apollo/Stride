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

namespace stride {
namespace popgen {

using uint = unsigned int;
using namespace std;
using namespace util;

class PopulationGenerator;

extern uniform_real_distribution<double> real01;

class SimplePerson {
public:

	SimplePerson(uint age=0, uint family_id=0);
	bool hasCommunitiesLeft();
	void print(std::ostream& os) const;

private:
	friend class PopulationGenerator;

	uint m_age = 0;
	uint m_household_id = 0;
	uint m_school_id = 0;
	uint m_work_id = 0;
	uint m_primary_community = 0;
	uint m_secondary_community = 0;
};

std::ostream& operator<<(std::ostream& os, const SimplePerson& p);


using Cluster = vector<uint>;

struct Population {
	vector<SimplePerson> all;
	vector<Cluster> families;
	vector<Cluster> schools;
	vector<Cluster> workplaces;
	vector<Cluster> communities;
};

struct SimpleSchool {
	uint m_current_pupils = 0;
	uint m_id = 0;
};

std::ostream& operator<<(std::ostream& os, const Population& p);

struct MinMax {
	MinMax(uint _min=0, uint _max=0): min(_min), max(_max) {}
	uint min;
	uint max;
};

struct MinMaxAvg: public MinMax {
	MinMaxAvg(uint _min=0, uint _max=0, uint _avg=0)
			: MinMax(_min, _max), avg(_avg) {}
	uint avg;
};

class AgeDistribution {
public:
	AgeDistribution(uint total=0, uint min=0, uint max=2, uint constant_up_to=1)
			: m_total(total), m_min(min), m_max(max), m_constant_up_to(constant_up_to),
			  m_first_pick(min, max) {
		assert(min < max);
		assert(min <= constant_up_to);
		assert(constant_up_to <= max);
	}

	void correct(uint total, const map<uint, uint>& age_counts);

	template<typename RNG>
	uint get(RNG& rng) {
		while (true) {
			uint age = m_first_pick(rng);
			if (age <= m_constant_up_to) return age;
			double declined_prob = double(age - m_constant_up_to) / double(m_max - m_constant_up_to);
			if (real01(rng) > declined_prob) return age;
		}
	}

	template<typename RNG>
	uint get_limited(RNG& rng, uint min, uint max) {
		assert(max >= m_min);
		assert(min <= m_max);
		assert(min <= max);
		if (max <= m_constant_up_to) {
			return uniform_int_distribution<uint>(min, max)(rng);
		} else {
			while (true) {  // TODO possible speedup by writing even more specialized code
				uint age = get(rng);
				if (min <= age and age <= max) return age;
			}
		}
	}

	template<typename RNG>
	uint get_limited(RNG& rng, const MinMax& mm) {
		return get_limited(rng, mm.min, mm.max);
	}

	inline uint getMin() const { return m_min; }

	inline uint getMax() const { return m_max; }

	inline uint getConstantUpTo() const { return m_constant_up_to; }

private:
	uint m_total;
	uint m_min;
	uint m_max;
	uint m_constant_up_to;
	uniform_int_distribution<uint> m_first_pick;
};

class PopulationGenerator {
public:

	PopulationGenerator(const string& filename);

	Population generate();

private:
	void makeSchools(const map<uint, uint>& age_map, Population& pop);

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

