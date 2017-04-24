#include "util/GeoCoordinate.h"
#include "util/GeoCoordCalculator.h"
#include "popgen/utils.h"
#include <trng/uniform_dist.hpp>

using namespace stride;
using namespace util;
using namespace std;

double earth_radius = 6371;
/// Mean radius of the earth (in metres)
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

template<typename T>
GeoCoordinate GeoCoordCalculator::generateRandomCoord(
		const GeoCoordinate& coord,
		double radius,
		RNGPicker<T>& rng) const {
	/// Partially the inverse of GeoCoordCalculator::getDistance, therefore i use the same variable names
	/// For future improvements, use this: http://gis.stackexchange.com/questions/25877/generating-random-locations-nearby
	double temp2 = radius / earth_radius;
	double temp1 = sin(temp2 / 2.0) * sin(temp2 / 2.0);

	double max_delta_latitude = asin(sqrt(temp1)) * 360.0 / PI;

	double my_cos = cos(coord.m_latitude * PI / 180.0);
	double my_pow = pow(my_cos, 2);
	double max_delta_longitude = asin(sqrt(temp1 / my_pow)) * 360.0 / PI;

	// std::uniform_real_distribution<double> dist_longitude(-max_delta_longitude, max_delta_longitude);
	// std::uniform_real_distribution<double> dist_latitude(-max_delta_latitude, max_delta_latitude);
	trng::uniform_dist<double> dist_longitude(-max_delta_longitude, max_delta_longitude);
	trng::uniform_dist<double> dist_latitude(-max_delta_latitude, max_delta_latitude);
	GeoCoordinate random_coordinate;

	do {
		double new_longitude = coord.m_longitude + dist_longitude(rng);
		double new_latitude = coord.m_latitude + dist_latitude(rng);
		random_coordinate.m_longitude = new_longitude;
		random_coordinate.m_latitude = new_latitude;
	} while (getDistance(coord, random_coordinate) > radius);
	return random_coordinate;
}
