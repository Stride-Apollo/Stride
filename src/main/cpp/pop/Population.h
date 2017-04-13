#pragma once
/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Header file for the core Population class
 */

#include "Person.h"
#include "core/Health.h"
#include "sim/Simulator.h"
#include "util/SimplePlanner.h"

#include <numeric>
#include <vector>

namespace stride {

using namespace std;
using namespace util;

class Population;

// Writing your own iterator 101
// Forward? Backward? Random access? Constness?
// PopT is either `Population` or `const Population`
template <typename PopT, typename IterT>
class _PopulationIterator {
protected:
	_PopulationIterator(PopT& pop, unsigned int index, bool in_planner = false, unsigned int day = 0)
			: m_pop(pop), m_in_planner(in_planner), m_index(index), m_day(day) {}

	friend class Population;

	void next() {
		m_index++;
		if (not m_in_planner) {
			if (m_index >= m_pop.m_original.size()) {
				m_in_planner = true;
				m_index = 0;
				m_day = 0;
				m_day_iter = m_pop.m_visitors.getAgenda().begin();
			}
		}
		if (m_in_planner) {
			while (true) {
				if (m_day >= m_pop.m_visitors.days()) {
					m_index = 0;
					break;
				} else if ((m_index >= m_pop.m_visitors.getDay(m_day)->size())) {
					m_day++;
					m_day_iter++;
					m_index = 0;
				} else {
					break;
				}
			}
		}
	}

	void trueNext() {
		if (isEnd()) {
			return;
		}
		while (true) {
			next();
			if (isEnd()) {
				return;
			}
			if (m_in_planner or (not (*(*this)).isOnVacation())) {
				return;
			}
		}
	}

public:

	void operator++(int) { trueNext(); }
	void operator++() { trueNext(); }

	bool operator== (const _PopulationIterator& other) const {
		return &m_pop == &(other.m_pop)
				and m_in_planner == other.m_in_planner
				and m_index == other.m_index
				and m_day == other.m_day;
	}

	bool operator!= (const _PopulationIterator& other) const {
		return not (*this == other);
	}

	const typename PopT::PersonType& operator*() const {
		if (m_in_planner) return (*m_day_iter->get()).at(m_index);
		else return m_pop.m_original.at(m_index);
	}

	bool isEnd() {
		return m_in_planner
			and m_index == 0
			and m_day == m_pop.m_visitors.days();
	}

protected:
	PopT& m_pop;
	bool m_in_planner;
	unsigned int m_index;  // when in planner, denotes the index within the day
	unsigned int m_day;  // not used when not in planner
	IterT m_day_iter;
};

class PopulationIterator;
class ConstPopulationIterator;

/**
 * Container for persons in population.
 */
class Population {
public:
	Population() = default;

	using PersonType = Simulator::PersonType;
	using PlannerType = SimplePlanner<PersonType>;
	using VectorType = vector<PersonType>;

	/// Get the cumulative number of cases.
	unsigned int getInfectedCount() const;

	PopulationIterator begin();
	PopulationIterator end();

	ConstPopulationIterator begin() const;
	ConstPopulationIterator end() const;

	// These are public, since otherwise I'd have to proxy literally every operation.
	VectorType m_original;
	PlannerType m_visitors;

	// standard library style
	using iterator = PopulationIterator;
	using const_iterator = ConstPopulationIterator;
};


using _PopIter = _PopulationIterator<Population, Population::PlannerType::Agenda::iterator>;
class PopulationIterator : public _PopIter {
public:
	friend class Population;
	using _PopIter::_PopulationIterator;
	Population::PersonType& operator*() const;
};


using _ConstPopIter = _PopulationIterator<const Population, Population::PlannerType::Agenda::const_iterator>;
class ConstPopulationIterator : public _ConstPopIter {
public:
	friend class Population;
	using _ConstPopIter::_PopulationIterator;
};


}
