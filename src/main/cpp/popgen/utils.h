#pragma once

#include <random>
#include <string>

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

class RandomGenerator {
public:
	virtual uint operator()() = 0;
};

class MinStdRand0: public RandomGenerator {
public:
	MinStdRand0 (uint seed);

	virtual uint operator()() override;

private:
	minstd_rand0 m_generator;
};

class MinStdRand: public RandomGenerator {
public:
	MinStdRand (uint seed);

	virtual uint operator()() override;

private:
	minstd_rand m_generator;
};

class MT19937: public RandomGenerator {
public:
	MT19937 (uint seed);

	virtual uint operator()() override;

private:
	mt19937 m_generator;
};

class MT19937_64: public RandomGenerator {
public:
	MT19937_64 (uint seed);

	virtual uint operator()() override;

private:
	mt19937_64 m_generator;
};

class Ranlux24: public RandomGenerator {
public:
	Ranlux24 (uint seed);

	virtual uint operator()() override;

private:
	ranlux24 m_generator;
};

class Ranlux48: public RandomGenerator {
public:
	Ranlux48 (uint seed);

	virtual uint operator()() override;

private:
	Ranlux48 m_generator;
};

class KnuthB: public RandomGenerator {
public:
	KnuthB (uint seed);

	virtual uint operator()() override;

private:
	knuth_b m_generator;
};


}
}
