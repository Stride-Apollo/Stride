#pragma once

#include <string>
#include "util/GeoCoordinate.h"
#include <random>

namespace stride {
namespace popgen {

using namespace std;
using namespace util;

#define PI 3.14159265

extern uniform_real_distribution<double> real01;

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

	SimpleCity(uint current_size, uint max_size, uint id, std::string name, util::GeoCoordinate coordinate) {
		m_current_size = current_size;
		m_max_size = max_size;
		m_id = id;
		m_name = name;
		m_coord = coordinate;
	}
	
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

}
}
