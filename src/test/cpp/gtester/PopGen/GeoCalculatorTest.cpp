/**
 * @file
 * Implementation of tests for the GeoCoordCalculator.
 */

#include "util/GeoCoordCalculator.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <map>
#include <string>
#include <tuple>
#include <random>
#include <iostream>

using namespace std;
using namespace stride;
using namespace util;
using namespace ::testing;

namespace Tests {

class GeoCalculatorDemos: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~GeoCalculatorDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
	static const GeoCoordinate                   g_coordinate;
	static const GeoCoordinate                   g_zero_lon;
	static const GeoCoordinate                   g_zero_lat;
	static const GeoCoordinate                   g_max_lat;
	static const GeoCoordinate                   g_min_lat;
	static const GeoCoordinate                   g_max_lon;
	static const GeoCoordinate                   g_min_lon;
	static const vector<uint>                    g_solutions;
	static const string                          g_output_prefix;
};

// Default values
const GeoCoordinate                  GeoCalculatorDemos::g_coordinate               = GeoCoordinate(43.21, 65.2);
const GeoCoordinate                  GeoCalculatorDemos::g_zero_lat                 = GeoCoordinate(0, -45.225);
const GeoCoordinate                  GeoCalculatorDemos::g_zero_lon                 = GeoCoordinate(85.145, 0);
const GeoCoordinate                  GeoCalculatorDemos::g_max_lat                  = GeoCoordinate(90.0, 15.142);
const GeoCoordinate                  GeoCalculatorDemos::g_min_lat                  = GeoCoordinate(-90.0, 15.142);
const GeoCoordinate                  GeoCalculatorDemos::g_max_lon                  = GeoCoordinate(-52.142, 180.0);
const GeoCoordinate                  GeoCalculatorDemos::g_min_lon                  = GeoCoordinate(-52.142, -180);

// Here follow the solutions between: 2 identical points, zero_lat and zero_lon, max_lat and min_lat, max_lon and min_lon and g_coordinate with any other point (in order of declaration)
// Note: Some calculated distances may differ from your calculator because of the radius of the earth
const vector<uint>                   GeoCalculatorDemos::g_solutions                = vector<uint> {0, 9627, 20015, 0, 11646, 4994, 5202, 14812, 15204, 15204};
const string                         GeoCalculatorDemos::g_output_prefix            = "GeoCalculator";

TEST_F(GeoCalculatorDemos, SingletonPattern_default) {
	// Test whether this is actually written according to the singleton pattern

	// -----------------------------------------------------------------------------------------
	// Actual tests
	// -----------------------------------------------------------------------------------------
	const GeoCoordCalculator& calc1 = GeoCoordCalculator::getInstance();
	const GeoCoordCalculator& calc2 = GeoCoordCalculator::getInstance();
	EXPECT_EQ(&calc1, &calc2);

	// -----------------------------------------------------------------------------------------
	// Release and close logger.
	// -----------------------------------------------------------------------------------------
	spdlog::drop_all();
}

TEST_F(GeoCalculatorDemos, distance) {
	// TODO: what is that logger?

	// -----------------------------------------------------------------------------------------
	// Get calculator
	// -----------------------------------------------------------------------------------------
	const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();

	// -----------------------------------------------------------------------------------------
	// Actual tests
	// -----------------------------------------------------------------------------------------
	EXPECT_EQ(uint(calc.getDistance(g_coordinate, g_coordinate)), g_solutions.at(0));
	EXPECT_EQ(uint(calc.getDistance(g_zero_lon, g_zero_lat)), g_solutions.at(1));
	EXPECT_EQ(uint(calc.getDistance(g_max_lat, g_min_lat)), g_solutions.at(2));
	EXPECT_EQ(uint(calc.getDistance(g_max_lon, g_min_lon)), g_solutions.at(3));

	for (uint i = 4; i < g_solutions.size(); i++) {
		GeoCoordinate current_coord;
		switch(i) {
			case 4:
				current_coord = g_zero_lat;
				break;
			case 5:
				current_coord = g_zero_lon;
				break;
			case 6:
				current_coord = g_max_lat;
				break;
			case 7:
				current_coord = g_min_lat;
				break;
			case 8:
				current_coord = g_max_lon;
				break;
			case 9:
				current_coord = g_min_lon;
				break;
		}

		EXPECT_EQ(uint(calc.getDistance(current_coord, g_coordinate)), g_solutions.at(i));
	}

	// This might seem trivial, but the calculation has to be independant of the coordinates
	EXPECT_EQ(calc.getDistance(g_coordinate, g_max_lon), calc.getDistance(g_max_lon, g_coordinate));
}

} //end-of-namespace-Tests
