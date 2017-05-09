#pragma once

#include <iostream>

#include "util/RNGPicker.h"
#include "util/GeoCoordinate.h"

namespace stride {
namespace util {

using namespace std;

class GeoCoordCalculator {
	/// Singleton pattern
public:
	static const GeoCoordCalculator& getInstance();

	double getDistance(const GeoCoordinate& coord1, const GeoCoordinate& coord2) const;
	/// Result is in kilometres
	/// Uses the haversine formula
	/// See: http://www.movable-type.co.uk/scripts/latlong.html

	GeoCoordinate generateRandomCoord(const GeoCoordinate& GeoCoordinate, double radius, RNGPicker& rng) const;
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