#pragma once

#include <random>
#include <string>

namespace stride {
namespace popgen {

using namespace std;

#define PI 3.14159265

extern uniform_real_distribution<double> real01;

using uint = unsigned int;

class PopulationGenerator;

struct GeoCoordinate {
	double m_longitude;
	double m_latitude;
};

class SimplePerson {
public:

	SimplePerson(uint age=0, uint family_id=0);

	friend std::ostream& operator<<(std::ostream& os, const SimplePerson& p);

private:
	friend class PopulationGenerator;

	uint m_age = 0;
	uint m_household_id = 0;
	uint m_school_id = 0;
	uint m_work_id = 0;
	uint m_primary_community = 0;
	uint m_secondary_community = 0;
	GeoCoordinate m_coord;
};

struct SimpleHousehold {
	uint m_id = 0;
	vector<uint> m_indices;
};

struct SimpleCluster {
	uint m_current_size = 0;
	uint m_max_size = 0;
	uint m_id = 0;
	GeoCoordinate m_coord;
};

struct SimpleCity {
	uint m_current_size = 0;
	uint m_max_size = 0;
	uint m_id = 0;
	string m_name = "";
	GeoCoordinate m_coord;
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
	using result_type = unsigned int;

	virtual result_type operator()() = 0;

	virtual result_type min() = 0;

	virtual result_type max() = 0;
};

class MinStdRand0: public RandomGenerator {
public:
	MinStdRand0 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	minstd_rand0 m_generator;
};

class MinStdRand: public RandomGenerator {
public:
	MinStdRand (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	minstd_rand m_generator;
};

class MT19937: public RandomGenerator {
public:
	MT19937 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	mt19937 m_generator;
};

class MT19937_64: public RandomGenerator {
public:
	MT19937_64 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	mt19937_64 m_generator;
};

class Ranlux24: public RandomGenerator {
public:
	Ranlux24 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	ranlux24 m_generator;
};

class Ranlux48: public RandomGenerator {
public:
	Ranlux48 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	std::discard_block_engine<std::ranlux48_base, 389, 11> m_generator;
};

class KnuthB: public RandomGenerator {
public:
	KnuthB (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return m_generator.min();}

	virtual result_type max() override {return m_generator.max();}

private:
	knuth_b m_generator;
};

class RNGPicker {
public:
	using result_type = unsigned int;

	RNGPicker();

	void set(string generator_type, result_type seed);

	~RNGPicker();

	result_type operator()();

	result_type min();

	result_type max();

private:
	RandomGenerator* m_rng;
};

std::ostream& operator<<(std::ostream& os, const GeoCoordinate& g);

class GeoCoordCalculator {
	/// Singleton pattern
public:
	static const GeoCoordCalculator& getInstance();

	double getDistance(const GeoCoordinate& coord1, const GeoCoordinate& coord2) const;
	/// Result is in kilometres
	/// Uses the haversine formula
	/// See: http://www.movable-type.co.uk/scripts/latlong.html

	GeoCoordinate generateRandomCoord(const GeoCoordinate& GeoCoordinate, double radius, RNGPicker& rng) const;
	/// radius is in kilometres
	/// TODO make the distribution fair

public:
	GeoCoordCalculator(){}

	~GeoCoordCalculator(){}

	GeoCoordCalculator(GeoCoordCalculator const&) = delete;
	void operator=(GeoCoordCalculator const&)  = delete;
};

}
}
