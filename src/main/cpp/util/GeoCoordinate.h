#pragma once

#include <iostream>

namespace stride {
namespace util {

using namespace std;

struct GeoCoordinate {
	// Note that it is required (precondition) to have valid long- and latitudes
	GeoCoordinate(){}
	GeoCoordinate(double lat, double lon) {m_latitude = lat; m_longitude = lon;}
	double m_longitude = 0.0;
	double m_latitude = 0.0;
};

bool operator==(const GeoCoordinate& coord1, const GeoCoordinate& coord2);

std::ostream& operator<<(std::ostream& os, const GeoCoordinate& g);

}
}