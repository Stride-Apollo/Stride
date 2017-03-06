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
 * I forgot this: make sure you can choose the rng and the seed
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


	static const string         g_invalid_input;
	static const string         g_nonexistent_input;
	static const string         g_empty_input;
	static const string         g_invalid_range;
	static const string         g_invalid_fraction;
	static const string         g_negative_numbers;
	static const string         g_missing_cluster;
	static const string         g_missing_family;
	static const string         g_missing_institution;
	static const string         g_invalid_rng;
	static const string         g_invalid_seed;
	static const string         g_invalid_element;
	static const string         g_output_file;
	static const string         g_happy_day_input;
	static const string         g_happy_day_expected;

	static PopulationGenerator 			g_generator;
};

// Default values
// TODO: make these files
const string         PopulationGeneratorTest::g_invalid_input               = "invalidFile.xml";
const string         PopulationGeneratorTest::g_nonexistent_input           = "doesntExist.xml";
const string         PopulationGeneratorTest::g_empty_input                 = "empty.xml";
const string         PopulationGeneratorTest::g_invalid_range               = "invalidRange.xml";
const string         PopulationGeneratorTest::g_invalid_fraction            = "invalidFraction.xml";
const string         PopulationGeneratorTest::g_negative_numbers            = "negativeNumbers.xml";
const string         PopulationGeneratorTest::g_missing_cluster             = "missingCluster.xml";
const string         PopulationGeneratorTest::g_missing_family              = "missingFamilySize.xml";
const string         PopulationGeneratorTest::g_missing_institution         = "missingInstitution.xml";
const string         PopulationGeneratorTest::g_invalid_rng                 = "invalidRng.xml";
const string         PopulationGeneratorTest::g_invalid_seed                = "invalidSeed.xml";
const string         PopulationGeneratorTest::g_invalid_element             = "invalidElement.xml";
const string         PopulationGeneratorTest::g_output_file                 = "output.csv";
const string         PopulationGeneratorTest::g_happy_day_input             = "happyDay.xml";
const string         PopulationGeneratorTest::g_happy_day_expected          = "happyDay.csv";

TEST_F(PopulationGeneratorTest, FileErrors) {
	// TODO write InvalidFileException (currently writing the tests before we write the stuff)
	// Test for file errors: non existent file, syntax errors

	EXPECT_THROW(g_generator.parseAndGenerate(g_invalid_file, g_output_file), InvalidFileException);
	EXPECT_THROW(g_generator.parseAndGenerate(g_nonexistent_input, g_output_file), NonExistentFileException);
}

TEST_F(PopulationGeneratorTest, HappyDay) {
	// Tests for the happy day scenario

	g_generator.parseAndGenerate(g_happy_day_input, g_output_file);

	// TODO write stuff that compares two files
		// and test that the files g_output_file and g_happy_day_expected are the same
}

TEST_F(PopulationGeneratorTest, WrongNextElement) {
	// TODO make the xml (where an unexpected element occurs) => do we ignore it and continue? do we throw an exception
	// Tests for when an unexpected element occurs
	EXPECT_THROW(g_generator.parseAndGenerate(g_invalid_element, g_output_file), InvalidElementException);
}

TEST_F(PopulationGeneratorTest, EmptyFile) {
	// TODO make the xml
	EXPECT_THROW(g_generator.parseAndGenerate(g_empty_input, g_output_file), InvalidFileException);
}

TEST_F(PopulationGeneratorTest, InvalidRanges) {
	// TODO make the xml and the exception
	// Tests for when the ranges are incorrect (e.g. age goes from min. 20 to max. 5)
	EXPECT_THROW(g_generator.parseAndGenerate(g_invalid_range, g_output_file), InvalidRangeException);
}

TEST_F(PopulationGeneratorTest, NegativeNumbers) {
	// TODO make the xml and the exception
	// Tests for when a negative number occurs, something which is not allowed
	EXPECT_THROW(g_generator.parseAndGenerate(g_negative_numbers, g_output_file), NegativeNumberException);
}

TEST_F(PopulationGeneratorTest, FractionErrors) {
	// TODO make the xml and the exception
	// Tests for when the total population exceeds 100% or something like that
	EXPECT_THROW(g_generator.parseAndGenerate(g_invalid_fraction, g_output_file), InvalidFractionException);
}

TEST_F(PopulationGeneratorTest, RandomTest) {
	// TODO make the xml and the exception
	// Tests for the random generator and the seed
	EXPECT_THROW(g_generator.parseAndGenerate(g_invalid_rng, g_output_file), InvalidRngException);
	EXPECT_THROW(g_generator.parseAndGenerate(g_invalid_seed, g_output_file), InvalidRngException);
}

TEST_F(PopulationGeneratorTest, ElementMissingErrors) {
	// TODO make the xml and the exception
	// Tests for when elements are missing, the important ones are:
		// <CLUSTER> within <CLUSTERS>
		// <SIZE> within <FAMILYSIZE>
		// <INSTITUTION> within <EDUCATION>
	EXPECT_THROW(g_generator.parseAndGenerate(g_missing_cluster, g_output_file), ElementMissingException);
	EXPECT_THROW(g_generator.parseAndGenerate(g_missing_institution, g_output_file), ElementMissingException);
	EXPECT_THROW(g_generator.parseAndGenerate(g_missing_family, g_output_file), ElementMissingException);
}

} //end-of-namespace-Tests