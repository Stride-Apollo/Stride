
#include "utils.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace stride;
using namespace popgen;

uniform_real_distribution<double> popgen::real01 = uniform_real_distribution<double>(0, 1);

bool SimplePerson::hasCommunitiesLeft() {
	return (m_primary_community != 0) and (m_secondary_community != 0);
}

std::ostream& popgen::operator<<(std::ostream& os, const SimplePerson& p) {
	assert(p.m_household_id != 0);  // everyone is part of a family!

	// "age","household_id","school_id","work_id","primary_community","secondary_community"
	os << p.m_age << "," << p.m_household_id << ","
	   << p.m_school_id << "," << p.m_work_id << ","
	   << p.m_primary_community << "," << p.m_secondary_community << "\n";

	return os;
}

SimplePerson::SimplePerson(uint age, uint family_id) :
		m_age(age), m_household_id(family_id) {
}

MinStdRand0::MinStdRand0(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type MinStdRand0::operator ()() {
	return m_generator();
}

MinStdRand::MinStdRand(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type MinStdRand::operator ()() {
	return m_generator();
}

MT19937::MT19937(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type MT19937::operator ()() {
	return m_generator();
}

MT19937_64::MT19937_64(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type MT19937_64::operator ()() {
	return m_generator();
}

Ranlux24::Ranlux24(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type Ranlux24::operator ()() {
	return m_generator();
}

Ranlux48::Ranlux48(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type Ranlux48::operator ()() {
	return m_generator();
}

KnuthB::KnuthB(RandomGenerator::result_type seed): m_generator{seed} {}

RandomGenerator::result_type KnuthB::operator ()() {
	return m_generator();
}

RNGPicker::RNGPicker(): m_rng{nullptr} {}

void RNGPicker::set(string generator_type, RandomGenerator::result_type seed) {
	if (m_rng != nullptr) {
		delete m_rng;
		m_rng = nullptr;
	}

	if (generator_type == "MinStdRand0")
		m_rng = new MinStdRand0(seed);
	else if (generator_type == "MinStdRand")
		m_rng = new MinStdRand(seed);
	else if (generator_type == "MT19937")
		m_rng = new MT19937(seed);
	else if (generator_type == "MT19937_64")
		m_rng = new MT19937_64(seed);
	else if (generator_type == "Ranlux24")
		m_rng = new Ranlux24(seed);
	else if (generator_type == "Ranlux48")
		m_rng = new Ranlux48(seed);
	else if (generator_type == "KnuthB")
		m_rng = new KnuthB(seed);
	else
		throw invalid_argument("Invalid RNG name");
}

RNGPicker::~RNGPicker() {
	if (m_rng != nullptr)
		delete m_rng;
}

RNGPicker::result_type RNGPicker::operator ()() {
	/// TODO write exception
	if (m_rng == nullptr)
		cerr << "rng not defined" << endl;
	else
		return (*m_rng)();
}

RNGPicker::result_type RNGPicker::min() {
	/// TODO throw exception
	if (m_rng != nullptr)
		return m_rng->min();
	else
		cerr << "Invalid rng\n";
	return 0;
}

RNGPicker::result_type RNGPicker::max() {
	/// TODO throw exception
	if (m_rng != nullptr)
		return m_rng->max();
	else
		cerr << "Invalid rng\n";
	return 0;
}
