#pragma once

#include <random>

namespace stride {
namespace popgen {

using namespace std;

extern uniform_real_distribution<double> real01;

using uint = unsigned int;

class PopulationGenerator;

class SimplePerson {
public:

	SimplePerson(uint age=0, uint family_id=0);
	bool hasCommunitiesLeft();

	friend std::ostream& operator<<(std::ostream& os, const SimplePerson& p);

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


}
}
