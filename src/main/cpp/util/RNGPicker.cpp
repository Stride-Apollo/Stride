#include "util/RNGPicker.h"
#include <cassert>

using namespace std;
using namespace stride;
using namespace util;

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
	if (m_rng == nullptr)
		throw runtime_error("Random number generator not defined");
	else
		return (*m_rng)();
	return 0;
}

RNGPicker::result_type RNGPicker::min() {
	if (m_rng != nullptr)
		return m_rng->min();
	else
		throw runtime_error("Random number generator not defined");
	return 0;
}

RNGPicker::result_type RNGPicker::max() {
	if (m_rng != nullptr)
		return m_rng->max();
	else
		throw runtime_error("Random number generator not defined");
	return 0;
}