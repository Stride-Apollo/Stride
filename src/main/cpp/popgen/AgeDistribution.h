#pragma once

#include <cassert>
#include <limits>

#include "util/AliasDistribution.h"

#include "utils.h"

namespace stride {
namespace popgen {

using uint = unsigned int;
using namespace std;
using namespace util;

class AgeDistribution {
public:
	AgeDistribution(uint total = 0, uint min = 0, uint max = 2, uint constant_up_to = 1);

	void correct(uint current_total, const map<uint, uint>& age_counts);

	MappedAliasDistribution get_dist(uint min = numeric_limits<uint>::min(),
									 uint max = numeric_limits<uint>::max());

	MappedAliasDistribution get_dist(const MinMax& mm);

	inline uint getMin() const { return m_min; }

	inline uint getMax() const { return m_max; }

	inline uint getConstantUpTo() const { return m_constant_up_to; }

private:
	uint m_total;
	uint m_min;
	uint m_max;
	uint m_constant_up_to;
	map<uint, double> m_probs;
	map<uint, uint> m_goal_counts;
};

}
}
