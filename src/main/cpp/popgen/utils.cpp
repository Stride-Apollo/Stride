#include "popgen/utils.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace stride;
using namespace popgen;

trng::uniform01_dist<double> popgen::real01;

std::ostream& operator<<(std::ostream& os, const SimplePerson& p) {
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

// MinStdRand0::MinStdRand0(RandomGenerator::result_type seed) {
// 	m_generator = trng::lcg64(seed);
// }
// RandomGenerator::result_type MinStdRand0::operator ()() {
// 	return m_generator();
// }

// RandomGenerator::RandomGenerator(std::string generator_type, long unsigned int){
// 	if (generator_type == "MinStdRand0"){
// 		m_generator = trng::lcg64(seed);
// 	}
// }
// TODO add more RandomGenerators when the basic 1 works
//
// MinStdRand::MinStdRand(RandomGenerator::result_type seed): m_generator{seed} {}
//
// RandomGenerator::result_type MinStdRand::operator ()() {
// 	return m_generator();
// }
//
// MT19937::MT19937(RandomGenerator::result_type seed): m_generator{seed} {}
//
// RandomGenerator::result_type MT19937::operator ()() {
// 	return m_generator();
// }
//
// MT19937_64::MT19937_64(RandomGenerator::result_type seed): m_generator{seed} {}
//
// RandomGenerator::result_type MT19937_64::operator ()() {
// 	return m_generator();
// }

// RNGPicker::RNGPicker(): m_rng{nullptr} {}
// template <class T>
// RNGPicker<T>::RNGPicker(long unsigned int seed) {
// 	m_rng = T(seed);
// }
template <typename T>
void RNGPicker<T>::set(std::string generator_type, long unsigned int seed) {
	if (m_rng != nullptr) {
		delete m_rng;
		m_rng = nullptr;
	}

	if (generator_type == "MinStdRand0")
		m_rng = T(seed);
	// else if (generator_type == "MinStdRand")
	// 	m_rng = new MinStdRand(seed);
	// else if (generator_type == "MT19937")
	// 	m_rng = new MT19937(seed);
	// else if (generator_type == "MT19937_64")
	// 	m_rng = new MT19937_64(seed);
	else
		throw std::invalid_argument("Invalid RNG name");
}

template <typename T>
RNGPicker<T>::~RNGPicker() {
	if (m_rng != nullptr)
		delete m_rng;
}

template <typename T>
long unsigned int RNGPicker<T>::operator ()() {
	if (m_rng == nullptr)
		throw std::runtime_error("Random number generator not defined");
	else
		return (*m_rng)();
	return 0;
}

// constexpr RNGPicker::result_type RNGPicker::min() {
// 	if (m_rng == nullptr)
// 		throw std::runtime_error("Random number generator not defined");
// 	else
// 		return m_rng->min();
// 	return 0;
// }

// constexpr RNGPicker::result_type RNGPicker::max() {
// 	if (m_rng != nullptr)
// 		return m_rng->max();
// 	else
// 		throw std::runtime_error("Random number generator not defined");
// 	return 0;
// 	return 0;
// }
