
#include "util/AliasDistribution.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <boost/math/distributions/chi_squared.hpp>

#include <vector>
#include <utility>
#include <random>

namespace Tests {

using namespace std;
using namespace stride;
using namespace util;
using namespace ::testing;

random_device rd;
mt19937 rng_mt = mt19937(123456);
// Big value for the confidence, because we'd rather see whether the distribition is certainly
// NOT correct, rather than certainly correct
const double confidence = 0.975;

bool chi_sq_test(vector<pair<uint, double>> observed_vs_theoretical, double confidence) {
	// See http://www.cse.wustl.edu/~jain/cse567-08/ftp/k_27trg.pdf for the maths behind this
	// Or you could also use the course "Elementaire Statistiek" by Florence Guillame (University of Antwerp)
	// First in the pair is the observed frequency, second is the theoretical frequency

	// TODO are these tests ok?
	double degrees_of_freedom = double(observed_vs_theoretical.size()) - 1.0;
	double D = 0;

	for (pair<uint, double>& my_pair: observed_vs_theoretical) {
		// Note how D is always positive
		if (my_pair.second != 0.0) {
			D += pow(my_pair.first - my_pair.second, 2) / my_pair.second;
		}
	}

	if (D == 0.0) {
		// It's a perfect fit!
		return true;
	}

	boost::math::chi_squared chisq {degrees_of_freedom};
	// 1 - p_value = probability of getting something "as extreme" as the given observations
	double p_value = boost::math::cdf(chisq, D);

	return p_value <= confidence;
}

vector<pair<uint, double>> run_alias_distribution(AliasDistribution& dist, const vector<double>& possible_observations,
												  mt19937& rnd, uint amount = 1000000) {
	vector<pair<uint, double>> observed_vs_theoretical = vector<pair<uint, double>>(possible_observations.size());

	// factor for rescaling (because the probabilities might not add up to 1)
	double factor = 1.0 / std::accumulate(possible_observations.begin(), possible_observations.end(), 0.0);

	// Fill theoretical observations and set the actual observations to 0
	for (uint i = 0; i < possible_observations.size(); i++) {
		observed_vs_theoretical.at(i).second = possible_observations.at(i) * amount * factor;
		observed_vs_theoretical.at(i).first = 0;
	}

	for (uint i = 0; i < amount; i++) {
		uint result = dist(rnd);
		observed_vs_theoretical.at(result).first++;
	}

	return observed_vs_theoretical;
}

TEST(AliasDistribution, Happyday__Normal) {
	// Test the normal distribution
	vector<double> prob_normal = {0.24, 0.26, 0.01, 0.09, 0.33, 0.07};
	AliasDistribution alias_normal = AliasDistribution(prob_normal);
	vector<pair<uint, double>> normal_result = run_alias_distribution(alias_normal, prob_normal, rng_mt);
	EXPECT_TRUE(chi_sq_test(normal_result, confidence));
}

TEST(AliasDistribution, Happyday__Big) {
	// Test the big distribution
	vector<double> prob_big = {0.48, 0.52, 0.02, 0.18, 0.66, 0.14};
	AliasDistribution alias_big = AliasDistribution(prob_big);
	vector<pair<uint, double>> big_result = run_alias_distribution(alias_big, prob_big, rng_mt);
	EXPECT_TRUE(chi_sq_test(big_result, confidence));
}

TEST(AliasDistribution, Boundaries__Zero) {
	AliasDistribution alias_zero({0.0});
	EXPECT_EQ(alias_zero(rd), 0);
	EXPECT_EQ(alias_zero(rng_mt), 0);
}

TEST(AliasDistribution, Boundaries__NearZero) {
	// Near zero
	vector<double> prob_near_zero = {0.0, 0.0, 0.0, 0.0, 0.01};
	AliasDistribution alias_near_zero(prob_near_zero);
	vector<pair<uint, double>> near_zero_result = run_alias_distribution(alias_near_zero, prob_near_zero, rng_mt);
	EXPECT_TRUE(chi_sq_test(near_zero_result, 0.0));

}

#ifdef NDEBUG
TEST(AliasDistribution, Boundaries__None) {
	// Empty vector is not allowed, assert should catch this
	EXPECT_DEATH(AliasDistribution(vector<double>()), ".*");
}
#endif

}
