/**
 * @file
 * Implementation of tests for the GeoCoordCalculator.
 */

#include "popgen/utils.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <map>
#include <string>
#include <tuple>
#include <random>
#include <iostream>

using namespace std;
using namespace stride;
using namespace popgen;
using namespace ::testing;

namespace Tests {

class GeoCalculatorDemos: public ::testing::Test
{
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
  static const vector<double>&    g_probabilities;
  static const vector<double>&    g_zero_probabilities;
  static const string             g_output_prefix;
};

// Default values
const vector<double>& GeoCalculatorDemos::g_probabilities                 = {0.24,0.26, 0.01, 0.09, 0.33, 0.07};
const vector<double>& GeoCalculatorDemos::g_zero_probabilities            = {0.0};
const string          GeoCalculatorDemos::g_output_prefix                 = "GeoCalculator";

TEST_F(GeoCalculatorDemos, SingletonPattern) {
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
  

  // -----------------------------------------------------------------------------------------
  // Release and close logger.
  // -----------------------------------------------------------------------------------------
  spdlog::drop_all();
  }

} //end-of-namespace-Tests
