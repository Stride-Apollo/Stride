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
