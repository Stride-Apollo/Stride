/**
 * @file
 * Implementation of tests for the alias distribution.
 */

#include "popgen/utils.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <boost/math/distributions/chi_squared.hpp>

#include <map>
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

class RNGPickerDemos: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~RNGPickerDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
	static const string              g_output_prefix;
};

// Default values
const string            RNGPickerDemos::g_output_prefix             = "AliasDistribution";

TEST_F(RNGPickerDemos, HappyDay_default) {
	// Tests which reflect the regular use

	// -----------------------------------------------------------------------------------------
	// Actual tests
	// -----------------------------------------------------------------------------------------
	RNGPicker rng;
	uint seed = 123456789;
	
	// mt19937
	rng.set("MT19937", seed);
	EXPECT_EQ(rng(), (mt19937(seed)) ());
	
	// mt19937
	rng.set("MinStdRand0", seed);
	EXPECT_EQ(rng(), (minstd_rand0(seed)) ());
	
	// mt19937
	rng.set("MinStdRand", seed);
	EXPECT_EQ(rng(), (minstd_rand(seed)) ());
	
	// mt19937
	rng.set("MT19937_64", seed);
	EXPECT_EQ(rng(), (mt19937_64(seed)) ());
	
	// mt19937
	rng.set("Ranlux24", seed);
	EXPECT_EQ(rng(), (ranlux24(seed)) ());
	
	// mt19937
	rng.set("Ranlux48", seed);
	EXPECT_EQ(rng(), (std::discard_block_engine<std::ranlux48_base, 389, 11>(seed)) ());
	
	// mt19937
	rng.set("KnuthB", seed);
	EXPECT_EQ(rng(), (knuth_b(seed)) ());
	
}

TEST_F(RNGPickerDemos, UnhappyDay_default) {
	// Tests on certain values given to the constructor of the AliasDistribution (e.g. empty vector of probabilities)


}

} //end-of-namespace-Tests
