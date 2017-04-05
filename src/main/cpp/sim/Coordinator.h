#pragma once

#include <vector>

#include "AsyncSimulator.h"

namespace stride {

using namespace std;

class Coordinator {
public:
	template <typename Iter>
	Coordinator(const Iter& sims)
		: m_sims(sims.begin(), sims.end()) {}

	Coordinator(initializer_list<AsyncSimulator*> sims)
		: m_sims(sims.begin(), sims.end()) {}

	void timeStep();

private:
	vector<AsyncSimulator*> m_sims;
};

}
