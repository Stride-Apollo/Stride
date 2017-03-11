
#include "AgeDistribution.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace stride;
using namespace popgen;
using namespace util;

uint round_uint(double d) {
	return (uint) d + 0.5;
}

AgeDistribution::AgeDistribution(uint total, uint min, uint max, uint constant_up_to)
		: m_total(total), m_min(min), m_max(max), m_constant_up_to(constant_up_to) {
	assert(min < max);
	assert(min <= constant_up_to);
	assert(constant_up_to <= max);

	// h is the 'height' of the rectangle (constant part) and triangle
	// (linear decrease part) forming the wanted age distribution.
	double h = 1.0/(double(constant_up_to - min) + double(max - constant_up_to)/2.0);
	uint age=min;
	for (; age<=constant_up_to; age++) {
		m_probs[age] = h;
	}
	for (; age<=max; age++) {
		m_probs[age] = h * double(max - age + 1)/double(max - constant_up_to + 1);
	}
	for (auto& it: m_probs) m_goal_counts[it.first] = round_uint(it.second * total);
}

void AgeDistribution::correct(uint current_total, const map<uint, uint>& age_counts) {
	for (auto& it: m_goal_counts) {
		uint age = it.first;
		uint goal_count = it.second;
//		cout << age << "\t" << m_probs[age] << "\t" << age_counts.at(age) << "\t" << current_total << "\n";
		if (age_counts.at(age) >= goal_count) {
			m_probs[age] = -2.0;
		} else if (current_total >= m_total) {
			m_probs[age] = -1.0;
		} else {
			m_probs[age] = double(goal_count - age_counts.at(age)) / double(m_total - current_total);
		}
//		cout << age << "\t" << m_probs[age] << "\t" << age_counts.at(age) << "\t" << current_total << "\n";
	}

	if (current_total >= m_total) {
		// at this point we're really stretching it
		for (auto& it: m_probs) {
			it.second = it.second == -1.0 ? 1.0 : 0.5;
		}
	} else {
		using PairType = map<uint, uint>::value_type;
		auto comp = [](const PairType& a, const PairType& b) { return a.second < b.second; };
		double min = std::min_element(m_probs.begin(), m_probs.end(), comp)->second;

		for (auto& it: m_probs) {
			if (it.second == -2.0) {
				it.second = min / 2;
			}
		}
	}
}

MappedAliasDistribution AgeDistribution::get_dist(uint _min, uint _max) {
	_min = max(_min, m_min);
	_max = min(_max, m_max);

	map<uint, double> probs;
	for (uint i=_min; i<=_max; i++) {
		probs[i] = m_probs[i];
	}

	return MappedAliasDistribution(probs);
}

MappedAliasDistribution AgeDistribution::get_dist(const MinMax& mm) {
	return get_dist(mm.min, mm.max);
}
