
#include "utils.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace stride;
using namespace popgen;

uniform_real_distribution<double> popgen::real01 = uniform_real_distribution<double>(0, 1);

double earth_radius = 6371;
/// Mean radius of the earth (in metres)

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



std::ostream& stride::popgen::operator<<(std::ostream& os, const GeoCoordinate& g) {
	os << "(LATITUDE: " << g.m_latitude << ", LONGITUDE: " << g.m_longitude << ")";
}

const GeoCoordCalculator& GeoCoordCalculator::getInstance() {
	static GeoCoordCalculator calc;

	return calc;
}

double GeoCoordCalculator::getDistance(const GeoCoordinate& coord1, const GeoCoordinate& coord2) const {
	double delta_latitude = coord2.m_latitude - coord1.m_latitude;
	double delta_longitude = coord2.m_longitude - coord1.m_longitude;

	double temp1 = sin(delta_latitude * PI / 360.0) * sin(delta_latitude * PI / 360.0) +
		cos(coord1.m_latitude * PI / 180.0) * cos(coord2.m_latitude * PI / 180.0) *
		sin(delta_longitude * PI / 360.0) * sin(delta_longitude * PI / 360.0);

	double temp2 = 2.0 * asin(min(1.0, sqrt(temp1)));

	return earth_radius * temp2;
}

GeoCoordinate GeoCoordCalculator::generateRandomCoord(
		const GeoCoordinate& coord,
		double radius,
		RNGPicker& rng) const {
	/// Partially the inverse of GeoCoordCalculator::getDistance, therefore i use the same variable names
	double temp2 = radius / earth_radius;
	double temp1 = sin(temp2 / 2.0) * sin(temp2 / 2.0);

	double max_delta_latitude = asin(sqrt(temp1)) * 360.0 / PI;

	double my_cos = cos(coord.m_latitude * PI / 180.0);
	double my_pow = pow(my_cos, 2);
	double max_delta_longitude = asin(sqrt(temp1 / my_pow)) * 360.0 / PI;

	std::uniform_real_distribution<double> dist_longitude(-max_delta_longitude, max_delta_longitude);
	std::uniform_real_distribution<double> dist_latitude(-max_delta_latitude, max_delta_latitude);
	GeoCoordinate random_coordinate;

	do {
		double new_longitude = coord.m_longitude + dist_longitude(rng);
		double new_latitude = coord.m_latitude + dist_latitude(rng);
		random_coordinate.m_longitude = new_longitude;
		random_coordinate.m_latitude = new_latitude;
	} while (getDistance(coord, random_coordinate) > radius);
	return random_coordinate;
}