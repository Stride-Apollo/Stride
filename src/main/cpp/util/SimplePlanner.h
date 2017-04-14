#pragma once

#include <list>
#include <vector>
#include <memory>
#include <iterator>

namespace stride {
namespace util {

using namespace std;

/**
 * You can see this class as a kind of planner that stores events in the near
 * future. You can then ask the planner what you should 'do' that day.
 * (Internally, it just saves a list of vectors of 'events' and does some
 * management to make sure there are enough vectors.)
 *
 * Used by Simulator for remembering when people have to return home from
 * their travels. (The 'event' is a traveller.)
 */
template <typename T>
class SimplePlanner {
public:
	using Block = vector<T>;
	using Agenda = list<unique_ptr<Block>>;

	Block* getModifiableDay(unsigned int days) {
		Block* block;
		if (days >= m_agenda.size()) {
			// make sure there are enough blocks in place
			uint new_blocks =  (days+1) - m_agenda.size();
			for (unsigned int i=0; i < new_blocks; i++) {
				block = new Block();
				m_agenda.emplace_back(block);
			}
		} else {
			block = next(m_agenda.begin(), days)->get();
		}
		return block;
	}

	const Block* getDay(unsigned int days) const {
		if (days >= m_agenda.size()) {
			// nothing planned, return an empty one
			return &g_empty_day;
		} else {
			return next(m_agenda.cbegin(), days)->get();
		}
	}

	const Block* today() const {
		return getDay(0);
	}

	void add(unsigned int days, T thing) {
		Block* block = getModifiableDay(days);
		block->push_back(thing);
	}

	void nextDay() {
		if (not m_agenda.empty()) m_agenda.pop_front();
	}

	unsigned int days() const {
		return m_agenda.size();
	}

	Agenda& getAgenda() {
		return m_agenda;
	}

	const Agenda& getAgenda() const {
		return m_agenda;
	}

private:
	Agenda m_agenda;
	static Block g_empty_day;
};

// look at how elegant C++ templates are
template <typename T>
typename SimplePlanner<T>::Block SimplePlanner<T>::g_empty_day = SimplePlanner<T>::Block();

}
}
