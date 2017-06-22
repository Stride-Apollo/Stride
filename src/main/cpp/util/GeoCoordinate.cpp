#include "util/GeoCoordinate.h"

using namespace stride;
using namespace util;
using namespace std;

bool stride::util::operator==(const GeoCoordinate& coord1, const GeoCoordinate& coord2) {
	return coord1.m_latitude == coord2.m_latitude && coord1.m_longitude == coord2.m_longitude;
}

bool stride::util::operator<(const GeoCoordinate& coord1, const GeoCoordinate& coord2) {
	if (coord1.m_longitude < coord2.m_longitude) {
		return true;
	} else if (coord1.m_longitude > coord2.m_longitude) {
		return false;
	}

	return coord1.m_latitude < coord2.m_latitude;
}

std::ostream& stride::util::operator<<(std::ostream& os, const GeoCoordinate& g) {
	os << "(LATITUDE: " << g.m_latitude << ", LONGITUDE: " << g.m_longitude << ")";
	return os;
}