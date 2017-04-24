#pragma once

#include <iostream>

#include "popgen/utils.h"

namespace stride {
namespace util {

using namespace std;
using namespace popgen;

class GeoCoordCalculator {
	/// Singleton pattern
public:
	static const GeoCoordCalculator& getInstance();

	double getDistance(const GeoCoordinate& coord1, const GeoCoordinate& coord2) const;
	/// Result is in kilometres
	/// Uses the haversine formula
	/// See: http://www.movable-type.co.uk/scripts/latlong.html

	template <typename T>
	GeoCoordinate generateRandomCoord(const GeoCoordinate& GeoCoordinate, double radius, RNGPicker<T>& rng) const;
	/// radius is in kilometres
	/// TODO make the distribution fair

/// TODO why is this public?
private:
	GeoCoordCalculator(){}

	~GeoCoordCalculator(){}

	GeoCoordCalculator(GeoCoordCalculator const&) = delete;
	void operator=(GeoCoordCalculator const&)  = delete;
};

}
}
