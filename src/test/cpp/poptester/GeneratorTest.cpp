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
 * Implementation of tests for the alias distribution.
 */

 #include "util/AliasDistribution.h"

 #include <gtest/gtest.h>
 #include <spdlog/spdlog.h>

 #include <map>
 #include <string>
 #include <tuple>
 #include <random>

 using namespace std;
 using namespace stride;
 using namespace util;
 using namespace ::testing;

namespace Tests {

class PopulationGeneratorDemos: public ::testing::Test
{
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~PopulationGeneratorDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
  static const vector<double>&    g_probabilities;
  static const string             g_output_prefix;
};

// Default values
const vector<double>& PopulationGeneratorDemos::g_probabilities           = {0.24,0.26, 0.01, 0.09, 0.33, 0.07};
const string          PopulationGeneratorDemos::g_output_prefix           = "AliasDistribution";

TEST_F(PopulationGeneratorDemos, AliasDistribution)
{
  // -----------------------------------------------------------------------------------------
  // initialize the logger.
  // -----------------------------------------------------------------------------------------
  spdlog::set_async_mode(1048576);
  auto file_logger = spdlog::rotating_logger_mt("contact_logger", g_output_prefix + "_logfile",
      std::numeric_limits<size_t>::max(),  std::numeric_limits<size_t>::max());
  file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

  auto Alias = AliasDistribution::AliasDistribution(g_probabilities);
  ASSERT_TRUE(true);

  // -----------------------------------------------------------------------------------------
  // Release and close logger.
  // -----------------------------------------------------------------------------------------
  spdlog::drop_all();
}

} //end-of-namespace-Tests
