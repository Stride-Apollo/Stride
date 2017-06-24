#pragma once

#include <iostream>
#include <random>

#include "util/GeoCoordinate.h"

namespace stride {
namespace util {

using namespace std;

class GeoCoordCalculator {
	/// Singleton pattern
public:
	static const GeoCoordCalculator& getInstance();

	double getDistance(const GeoCoordinate& coord1, const GeoCoordinate& coord2) const;
	/// Result is in kilometers
	/// Uses the haversine formula
	/// See: http://www.movable-type.co.uk/scripts/latlong.html

	template<class T>
	GeoCoordinate generateRandomCoord(
			const GeoCoordinate& coord,
			double radius,
			T rng) const {
		/// Partially the inverse of GeoCoordCalculator::getDistance, therefore I use the same variable names
		/// For future improvements, use this: http://gis.stackexchange.com/questions/25877/generating-random-locations-nearby
		double temp2 = radius / 6371;
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
	/// radius is in kilometres
	/// TODO make the distribution fair

private:
	GeoCoordCalculator(){}

	~GeoCoordCalculator(){}

	GeoCoordCalculator(GeoCoordCalculator const&) = delete;
	void operator=(GeoCoordCalculator const&)  = delete;
};

}
}
