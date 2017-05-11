/**
 * @file
 * Implementation of tests for the GeoCoordCalculator.
 */

#include "util/GeoCoordCalculator.h"

#include <gtest/gtest.h>

#include <map>
#include <string>
#include <random>
#include <iostream>

using namespace std;
using namespace stride;
using namespace util;
using namespace ::testing;

namespace Tests {

TEST(GeoCalculatorTest, SingletonPattern_default) {
	// Test whether this is actually written according to the singleton pattern

	const GeoCoordCalculator& calc1 = GeoCoordCalculator::getInstance();
	const GeoCoordCalculator& calc2 = GeoCoordCalculator::getInstance();
	EXPECT_EQ(&calc1, &calc2);
}

TEST(GeoCalculatorTest, distance_default) {
	auto solutions = vector<uint> {0, 9627, 20015, 0, 11646, 4994, 5202, 14812, 15204, 15204};
	const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();

	EXPECT_EQ(uint(calc.getDistance(GeoCoordinate(43.21, 65.2), GeoCoordinate(43.21, 65.2))), solutions.at(0));
	EXPECT_EQ(uint(calc.getDistance(GeoCoordinate(85.145, 0), GeoCoordinate(0, -45.225))), solutions.at(1));
	EXPECT_EQ(uint(calc.getDistance(GeoCoordinate(90.0, 15.142), GeoCoordinate(-90.0, 15.142))), solutions.at(2));
	EXPECT_EQ(uint(calc.getDistance(GeoCoordinate(-52.142, 180.0), GeoCoordinate(-52.142, -180))), solutions.at(3));

	for (uint i = 4; i < solutions.size(); i++) {
		GeoCoordinate current_coord;
		switch(i) {
			case 4:
				current_coord = GeoCoordinate(0, -45.225);
				break;
			case 5:
				current_coord = GeoCoordinate(85.145, 0);
				break;
			case 6:
				current_coord = GeoCoordinate(90.0, 15.142);
				break;
			case 7:
				current_coord = GeoCoordinate(-90.0, 15.142);
				break;
			case 8:
				current_coord = GeoCoordinate(-52.142, 180.0);
				break;
			case 9:
				current_coord = GeoCoordinate(-52.142, -180);
				break;
		}

		EXPECT_EQ(uint(calc.getDistance(current_coord, GeoCoordinate(43.21, 65.2))), solutions.at(i));
	}

	// This might seem trivial, but the calculation has to be independant of the coordinates
	EXPECT_EQ(calc.getDistance(GeoCoordinate(43.21, 65.2), GeoCoordinate(-52.142, 180.0)),
				calc.getDistance(GeoCoordinate(-52.142, 180.0), GeoCoordinate(43.21, 65.2)));
}

} //end-of-namespace-Tests
