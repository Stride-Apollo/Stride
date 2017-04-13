
#include "Population.h"

using namespace stride;
using namespace util;
using namespace std;

// Population
// ----------

unsigned int Population::getInfectedCount() const {
	unsigned int total = 0;
	for (const auto& p : *this) {
		const auto& h = p.getHealth();
		total += h.isInfected() || h.isRecovered();
	}
	return total;
}

#define PopulationBeginEnd(mod, type) \
type Population::begin() mod { \
	type it = type(*this, -1); \
	it++; \
	return it; \
} \
type Population::end() mod { \
	return type(*this, 0, true, m_visitors.days()); \
}

PopulationBeginEnd( , PopulationIterator)

PopulationBeginEnd(const, ConstPopulationIterator)


// PopulationIterator
// ------------------

Population::PersonType& PopulationIterator::operator*() const {
	if (m_in_planner) return (*m_day_iter->get())[m_index];
	else return m_pop.m_original[m_index];
}
