/**
 * @file
 * Implementation of tests for the family parser.
 */

#include "popgen/FamilyParser.h"
#include "util/InstallDirs.h"

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
using namespace util;

namespace Tests {

TEST(FamilyParserDemos, HappyDay_default) {
	// Tests which reflect the regular use

	auto solution = vector<FamilyConfig>{FamilyConfig {20, 18, 1},
											FamilyConfig {54, 52, 22, 20, 18},
											FamilyConfig {44,44,12,8},
											FamilyConfig {20},
											FamilyConfig {30, 29}};
	FamilyParser parser;

	EXPECT_EQ(parser.parseFamilies("happy_day.txt"), solution);
	EXPECT_TRUE(parser.parseFamilies("empty.txt").empty());
	EXPECT_EQ(parser.parseFamilies("whitespace.txt"), solution);
}

TEST(FamilyParserDemos, Failures_default) {
	FamilyParser parser;

	EXPECT_THROW(parser.parseFamilies("invalid_number.txt"), invalid_argument);
	EXPECT_THROW(parser.parseFamilies("invalid_delimiter.txt"), invalid_argument);
	EXPECT_THROW(parser.parseFamilies("negative_number.txt"), invalid_argument);
	EXPECT_THROW(parser.parseFamilies("non_existent.txt"), invalid_argument);
}

} //end-of-namespace-Tests
