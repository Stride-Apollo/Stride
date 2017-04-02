/**
 * @file
 * Implementation of tests for the family parser.
 */

#include "popgen/FamilyParser.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <string>
#include <tuple>
#include <random>
#include <iostream>
#include <utility>

using namespace std;
using namespace stride;
using namespace popgen;
using namespace ::testing;

namespace Tests {

class FamilyParserDemos: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~FamilyParserDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
	static const string                    g_happy_day_file;
	static const string                    g_empty_file;
	static const string                    g_whitespace_file;
	static const string                    g_invalid_delimiter_file;
	static const string                    g_invalid_number_file;
	static const string                    g_negative_number_file;
	static const string                    g_non_existent_file;
	static const vector<FamilyConfig>      g_solution;
	static const FamilyParser              g_parser;
	static const string                    g_output_prefix;
};

// Default values
const string                    FamilyParserDemos::g_happy_day_file                = "input/happy_day.txt";
const string                    FamilyParserDemos::g_empty_file                    = "input/empty.txt";
const string                    FamilyParserDemos::g_whitespace_file               = "input/whitespace.txt";
const string                    FamilyParserDemos::g_invalid_delimiter_file        = "input/invalid_delimiter.txt";
const string                    FamilyParserDemos::g_invalid_number_file           = "input/invalid_number.txt";
const string                    FamilyParserDemos::g_negative_number_file          = "input/negative_number.txt";
const string                    FamilyParserDemos::g_non_existent_file             = "input/non_existent.txt";
const vector<FamilyConfig>      FamilyParserDemos::g_solution                      = vector<FamilyConfig>{
																						FamilyConfig {20, 18, 1},
																						FamilyConfig {54, 52, 22, 20, 18},
																						FamilyConfig {44,44,12,8},
																						FamilyConfig {20},
																						FamilyConfig {30, 29}};
const FamilyParser              FamilyParserDemos::g_parser                        = FamilyParser();
const string                    FamilyParserDemos::g_output_prefix                 = "FamilyParser";


TEST_F(FamilyParserDemos, HappyDay) {
	// Tests which reflect the regular use

	// -----------------------------------------------------------------------------------------
	// Initialize the logger.
	// -----------------------------------------------------------------------------------------
	spdlog::set_async_mode(1048576);
	auto file_logger = spdlog::rotating_logger_mt("contact_logger", g_output_prefix + "_logfile",
		std::numeric_limits<size_t>::max(),  std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// -----------------------------------------------------------------------------------------
	// Actual tests
	// -----------------------------------------------------------------------------------------
	EXPECT_EQ(g_parser.parseFamilies(g_happy_day_file), g_solution);
	EXPECT_TRUE(g_parser.parseFamilies(g_empty_file).empty());
	EXPECT_EQ(g_parser.parseFamilies(g_whitespace_file), g_solution);

	// -----------------------------------------------------------------------------------------
	// Release and close logger.
	// -----------------------------------------------------------------------------------------
	spdlog::drop_all();
}

TEST_F(FamilyParserDemos, Failures) {
	EXPECT_THROW(g_parser.parseFamilies(g_invalid_number_file), invalid_argument);
	EXPECT_THROW(g_parser.parseFamilies(g_invalid_delimiter_file), invalid_argument);
	EXPECT_THROW(g_parser.parseFamilies(g_negative_number_file), invalid_argument);
	EXPECT_THROW(g_parser.parseFamilies(g_non_existent_file), invalid_argument);
}

} //end-of-namespace-Tests
