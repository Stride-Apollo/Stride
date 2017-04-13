#pragma once

#include <random>
#include <stdexcept>

namespace stride {
namespace util {

using namespace std;

class RandomGenerator {
public:
	virtual ~RandomGenerator(){}

	using result_type = long unsigned int;

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

	virtual result_type min() override {return result_type(m_generator.min());}

	virtual result_type max() override {return result_type(m_generator.max());}

private:
	minstd_rand m_generator;
};

class MT19937: public RandomGenerator {
public:
	MT19937 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return result_type(m_generator.min());}

	virtual result_type max() override {return result_type( m_generator.max());}

private:
	mt19937 m_generator;
};

class MT19937_64: public RandomGenerator {
public:
	MT19937_64 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return result_type(m_generator.min());}

	virtual result_type max() override {return result_type(m_generator.max());}

private:
	mt19937_64 m_generator;
};

class Ranlux24: public RandomGenerator {
public:
	Ranlux24 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return result_type(m_generator.min());}

	virtual result_type max() override {return result_type(m_generator.max());}

private:
	ranlux24 m_generator;
};

class Ranlux48: public RandomGenerator {
public:
	Ranlux48 (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return result_type(m_generator.min());}

	virtual result_type max() override {return result_type(m_generator.max());}

private:
	std::discard_block_engine<std::ranlux48_base, 389, 11> m_generator;
};

class KnuthB: public RandomGenerator {
public:
	KnuthB (result_type seed);

	virtual result_type operator()() override;

	virtual result_type min() override {return result_type(m_generator.min());}

	virtual result_type max() override {return result_type(m_generator.max());}

private:
	knuth_b m_generator;
};

class RNGPicker {
public:
	using result_type = long unsigned int;

	RNGPicker();

	void set(string generator_type, result_type seed);

	~RNGPicker();

	result_type operator()();

	result_type min();

	result_type max();

private:
	RandomGenerator* m_rng;
};

}
}