#pragma once

#include <string>
#include "util/GeoCoordinate.h"

// Distribution and random generator from the TRNG library
#include <trng/uniform01_dist.hpp>
#include <trng/lcg64.hpp>
#include <trng/lcg64_shift.hpp>
#include <trng/mt19937.hpp>
#include <trng/mt19937_64.hpp>

namespace stride {
namespace popgen {

// using namespace std;
// using namespace util;

#define PI 3.14159265

extern trng::uniform01_dist<double> real01;

using uint = unsigned int;

template<class T> class PopulationGenerator;

class SimplePerson {
public:

	SimplePerson(uint age=0, uint family_id=0);

	friend std::ostream& operator<<(std::ostream& os, const SimplePerson& p);
	template<class U> friend class PopulationGenerator;

	uint m_age = 0;
	uint m_household_id = 0;
	uint m_school_id = 0;
	uint m_work_id = 0;
	uint m_primary_community = 0;
	uint m_secondary_community = 0;
	util::GeoCoordinate m_coord;
};

struct SimpleHousehold {
	uint m_id = 0;
	std::vector<uint> m_indices;
};

struct SimpleCluster {
	uint m_current_size = 0;
	uint m_max_size = 0;
	uint m_id = 0;
	util::GeoCoordinate m_coord;
};

struct SimpleCity {
	uint m_current_size = 0;
	uint m_max_size = 0;
	uint m_id = 0;
	std::string m_name = "";
	util::GeoCoordinate m_coord;
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

// template<class T>
// class RandomGenerator {
// public:
// 	RandomGenerator(std::string, long unsigned int);
// 	~RandomGenerator() = default;
//
// 	using result_type = long unsigned int;
//
// 	result_type operator()();
//
// 	result_type min(){return m_generator.min();};
//
// 	result_type max(){return m_generator.max();};
//
// private:
// 	T m_generator;
//
// };

// class MinStdRand0: public RandomGenerator {
// public:
// 	MinStdRand0 (result_type seed);
//
// 	virtual result_type operator()() override;
//
// 	virtual result_type min() override {return m_generator.min();}
//
// 	virtual result_type max() override {return m_generator.max();}
//
// private:
// 	trng::lcg64 m_generator;
// };
template <class T>
class RNGPicker {
public:
	using result_type = long unsigned int;

	RNGPicker(){m_rng = nullptr;};

	void set(std::string generator_type, result_type seed);

	~RNGPicker();

	long unsigned int operator()();

  // TODO fix this
	constexpr static const result_type min() { return 0; }

	constexpr static const result_type max() { return 1000; }

private:
	static T* m_rng;
};

}
}
