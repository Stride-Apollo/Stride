#include "AliasDistribution.h"
#include <cassert>
#include <numeric>

using namespace stride;
using namespace util;

AliasDistribution::AliasDistribution(const vector<double>& _probs)
		: m_blocks(_probs.size()), m_diceroll(0, _probs.size()-1) {
	// Check if sum is (almost) equal to 1.0
	assert(abs(std::accumulate(_probs.begin(), _probs.end(), 0.0)-1.0) < 0.0001);

	unsigned int n = _probs.size();
	vector<double> probs(n);
	for (int i=0; i<n; i++) probs[i] = _probs[i] * n;

	deque<unsigned int> small, large;
	for (int i=0; i<n; i++) {
		(probs[i] < 1.0 ? small : large).push_back(i);
	}

	while ((not small.empty()) and (not large.empty())) {
		unsigned int l = *small.begin();
		small.pop_front();
		unsigned int g = *large.begin();
		large.pop_front();

		m_blocks[l].prob = probs[l];
		m_blocks[l].alias = g;
		probs[g] = (probs[g] + probs[l]) - 1;
		(probs[g] < 1.0 ? small : large).push_back(g);
	}

	for (unsigned int i: large) m_blocks[i].prob = 1.0;
	// If small is not empty, this may be sign of numerical instability
	for (unsigned int i: small) m_blocks[i].prob = 1.0;
}
