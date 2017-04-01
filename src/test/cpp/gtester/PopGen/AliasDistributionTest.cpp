/**
 * @file
 * Implementation of tests for the alias distribution.
 */

#include "util/AliasDistribution.h"

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
using namespace util;
using namespace ::testing;

namespace Tests {

class AliasDistributionDemos: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~AliasDistributionDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
	static const vector<double>      g_prob_normal;
	static const vector<double>      g_prob_big;
	static const vector<double>      g_prob_zero;
	static const vector<double>      g_prob_near_zero;
	static AliasDistribution        g_alias_normal;
	static AliasDistribution        g_alias_big;
	static AliasDistribution        g_alias_zero;
	static AliasDistribution        g_near_zero;
	static random_device             g_rd;
	static mt19937                   g_rng_mt;
	static const double			     g_confidence;
	static const uint                g_happy_day_amount;
	static const string              g_output_prefix;
};

// Default values
const vector<double>    AliasDistributionDemos::g_prob_normal               = vector<double>({0.24,0.26, 0.01, 0.09, 0.33, 0.07});
const vector<double>    AliasDistributionDemos::g_prob_big                  = vector<double>({0.48,0.52, 0.02, 0.18, 0.66, 0.14});
const vector<double>    AliasDistributionDemos::g_prob_zero                 = vector<double>({0.0});
const vector<double>    AliasDistributionDemos::g_prob_near_zero            = vector<double>({0.0, 0.0, 0.0, 0.0, 0.01});
AliasDistribution AliasDistributionDemos::g_alias_normal                    = AliasDistribution({0.24,0.26, 0.01, 0.09, 0.33, 0.07});
AliasDistribution AliasDistributionDemos::g_alias_big                       = AliasDistribution({0.48,0.52, 0.02, 0.18, 0.66, 0.14});
AliasDistribution AliasDistributionDemos::g_alias_zero                      = AliasDistribution({0.0});
AliasDistribution AliasDistributionDemos::g_near_zero                       = AliasDistribution({0.0, 0.0, 0.0, 0.0, 0.01});
random_device           AliasDistributionDemos::g_rd;
mt19937                 AliasDistributionDemos::g_rng_mt                    = mt19937(AliasDistributionDemos::g_rd());
const double            AliasDistributionDemos::g_confidence                = 0.975;
	// Big value for the confidence => see if the distribution is certainly NOT correct (instead of certainly correct)
const uint              AliasDistributionDemos::g_happy_day_amount          = 1000000;
const string            AliasDistributionDemos::g_output_prefix             = "AliasDistribution";

bool chi_sq_test(vector<pair<uint, double> > observed_vs_theoretical, double confidence) {
	/// See http://www.cse.wustl.edu/~jain/cse567-08/ftp/k_27trg.pdf for the maths behind this
	/// Or you could also use the course "Elementaire Statistiek" by Florence Guillame (University of Antwerp)
	/// First in the pair is the observed frequency, second is the theoretical frequency
	double degrees_of_freedom = double(observed_vs_theoretical.size()) - 1.0;
	double D = 0;

	for(pair<uint, double>& my_pair: observed_vs_theoretical) {
		/// Note how D is always positive
		D += pow(my_pair.first - my_pair.second, 2) / my_pair.second;
	}

	boost::math::chi_squared chisq {degrees_of_freedom};
	/// 1 - p_value = probability of getting something "as extreme" as the given observations
	double p_value = boost::math::cdf(chisq,D);

	if (p_value > confidence) {
		return false;
	} else {
		return true;
	}
}

vector<pair<uint, double> > run_alias_distribution(AliasDistribution& dist, uint amount, const vector<double>& possible_observations, mt19937& rnd) {
	vector<pair<uint, double> > observed_vs_theoretical = vector<pair<uint, double> >(possible_observations.size());

	// factor for rescaling (because the probabilities might not add up to 1)
	double factor = 1.0 / std::accumulate(possible_observations.begin(), possible_observations.end(), 0.0);

	/// Fill theoretical observations and set the actual observations to 0
	for (uint i = 0; i < possible_observations.size(); i++) {
		observed_vs_theoretical.at(i).second = possible_observations.at(i) * amount * factor;
		observed_vs_theoretical.at(i).first = 0;
	}

	for (uint i = 0; i < amount; i++) {
		uint result = dist.operator()<mt19937>(rnd);
		observed_vs_theoretical.at(result).first++;
	}

	return observed_vs_theoretical;
}

void death () {
	//assert(5 > 50);
}

TEST_F(AliasDistributionDemos, HappyDay) {
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
	// Test the normal distribution 
	vector<pair<uint, double> > normal_result = run_alias_distribution(g_alias_normal, g_happy_day_amount, g_prob_normal, g_rng_mt);
	EXPECT_TRUE(chi_sq_test(normal_result, g_confidence));

	// Test the big distribution
	vector<pair<uint, double> > big_result = run_alias_distribution(g_alias_big, g_happy_day_amount, g_prob_big, g_rng_mt);
	EXPECT_TRUE(chi_sq_test(big_result, g_confidence));

	// -----------------------------------------------------------------------------------------
	// Release and close logger.
	// -----------------------------------------------------------------------------------------
	spdlog::drop_all();
}

TEST_F(AliasDistributionDemos, Boundaries) {
	// Tests on certain values given to the constructor of the AliasDistribution (e.g. empty vector of probabilities)

	// Zero chances
	EXPECT_EQ(g_alias_zero.operator()<random_device>(g_rd), 0U);
	EXPECT_EQ(g_alias_zero.operator()<mt19937>(g_rng_mt), 0U);

	// TODO, adjust this in alias distribution
	// Empty vector
	//ASSERT_THROW(AliasDistribution(vector<double>()), invalid_argument);

	// Default constructor (expected to be the same as the empty vector)
	//ASSERT_THROW(AliasDistribution(), invalid_argument);

}

} //end-of-namespace-Tests
