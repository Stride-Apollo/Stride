
#include <list>
#include <vector>
#include <memory>

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
private:
	using Block = vector<T>;
	using Agenda = list<unique_ptr<Block>>;

public:
	Block* getModifiableDay(unsigned int days) {
		Block* block;
		if (days >= m_agenda.size()) {
			// make sure there are enough blocks in place
			for (unsigned int i=0; i<((days+1) - m_agenda.size()); i++) {
				block = new Block();
				m_agenda.emplace_back(block);
			}
		} else {
			block = *(m_agenda.begin()+days);
		}
		return block;
	}

	const Block* getDay(unsigned int days) const {
		if (days >= m_agenda.size()) {
			// nothing planned, return an empty one
			return &g_empty_day;
		} else {
			return *(m_agenda.cbegin()+days);
		}
	}

	void add(unsigned int days, T thing) {
		Block* block = getModifiableDay(days);
		block->push_back(thing);
	}

	void nextDay() {
		m_agenda.pop_front();
	}

private:
	Agenda m_agenda;
	static Block g_empty_day = Block();
};


}
}
