
#include "utils.h"

#include <cassert>
#include <iostream>

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

stride::popgen::MinStdRand0::MinStdRand0(uint seed): m_generator{seed} {}

uint stride::popgen::MinStdRand0::operator ()() {
	return m_generator();
}

stride::popgen::MinStdRand::MinStdRand(uint seed): m_generator{seed} {}

uint stride::popgen::MinStdRand::operator ()() {
	return m_generator();
}

stride::popgen::MT19937::MT19937(uint seed): m_generator{seed} {}

uint stride::popgen::MT19937::operator ()() {
	return m_generator();
}

stride::popgen::MT19937_64::MT19937_64(uint seed): m_generator{seed} {}

uint stride::popgen::MT19937_64::operator ()() {
	return m_generator();
}

stride::popgen::Ranlux24::Ranlux24(uint seed): m_generator{seed} {}

uint stride::popgen::Ranlux24::operator ()() {
	return m_generator();
}

stride::popgen::Ranlux48::Ranlux48(uint seed): m_generator{seed} {}

uint stride::popgen::Ranlux48::operator ()() {
	return m_generator();
}

stride::popgen::KnuthB::KnuthB(uint seed): m_generator{seed} {}

uint stride::popgen::KnuthB::operator ()() {
	return m_generator();
}
