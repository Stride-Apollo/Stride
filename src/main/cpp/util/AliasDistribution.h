#pragma once

#include <vector>
#include <deque>
#include <random>

namespace stride {
namespace util {

using namespace std;

struct AliasBlock {
	double prob;
	unsigned int alias;
};

/// Usage is very simple, construct with a vector of probabilities (with sum 1),
/// then use as a distribution from the standard library (i.e. with operator()).
class AliasDistribution {
public:
	/**
	 * Construct the distribution using the method described here:
	 * http://keithschwarz.com/darts-dice-coins/
	 *
	 * @param probs		A vector with length n, whose sum has to equal 1.
	 */
	AliasDistribution(const vector<double>& probs);

	/**
	 * @param gen		A random generator conforming the standard operator() usage
	 * @return			A random (weighted) integer in [0, n)
	 */
	template<typename RandGen>
	unsigned int operator()(RandGen& gen) {
		unsigned int i = m_diceroll(gen);
		double c = g_coinflip(gen);
		return c < m_blocks[i].prob ? i : m_blocks[i].alias;
	}

private:
	vector<AliasBlock> m_blocks;
	uniform_int_distribution<unsigned int> m_diceroll;
	static uniform_real_distribution<double> g_coinflip;
};

}
}
