/*
\ *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of scenario tests running in batch mode.
 */

/**
 * TODO
 * Make an XML format
 * Complete the tests according to the XML format
 * Make the PopulationGenerator class
 * Include the PopulationGenerator in this file
 * Include both the PopulationGenerator.cpp and this file in their CmakeLists
 * Run the tests and hopefully find lost and lots of errors
 */

#include "pop/Population.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"

#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>
#include <omp.h>
#include <spdlog/spdlog.h>

#include <cmath>
#include <map>
#include <string>
#include <tuple>

using namespace std;
using namespace stride;
using namespace ::testing;

namespace Tests {

class PopulationGeneratorTest: public ::testing::Test
{
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~PopulationGeneratorTest() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	/// Data members of the test fixture
	// TODO add data members while fully writing the tests
	static const string            g_invalid_input;
	static const string            g_output_file;
	static const string            g_happy_day_input;
	static const string            g_happy_day_expected;

	static PopulationGenerator 			g_generator;
};

// Default values
const string         PopulationGeneratorTest::g_invalid_input               = "doesntExist.xml";
const string         PopulationGeneratorTest::g_output_file                 = "output.csv";
const string         PopulationGeneratorTest::g_happy_day_input             = "happyDay.xml";
const string         PopulationGeneratorTest::g_happy_day_expected          = "happyDay.csv";

TEST_F(PopulationGeneratorTest, InvalidFile) {
	// TODO write InvalidFileException (currently writing the tests before we write the stuff)
	EXPECT_THROW(g_generator.parseFile(g_invalid_file, g_output_file), InvalidFileException);
}

TEST_F(PopulationGeneratorTest, HappyDay) {
	g_generator.parseFile(g_happy_day_input, g_output_file);

	// TODO write stuff that compares two files
		// and test that the files g_output_file and g_happy_day_expected are the same
}

TEST_F(PopulationGeneratorTest, InvalidFatalXML) {
	// TODO make the xml, then check where it can go wrong (lethal) => throw exceptions
}

TEST_F(PopulationGeneratorTest, InvalidNonFatalXML) {
	// TODO make the xml, then check where it can go wrong (non-lethal) => throw exceptions but resolve them yourself
}

} //end-of-namespace-Tests