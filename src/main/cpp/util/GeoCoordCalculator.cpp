#include "util/GeoCoordinate.h"
#include "util/GeoCoordCalculator.h"

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
