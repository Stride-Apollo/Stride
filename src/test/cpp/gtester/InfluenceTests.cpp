/**
 * @file
 * Implementation of tests for the Influence class.
 */

#include <gtest/gtest.h>

#include <cmath>
#include <algorithm>

#include "core/Influence.h"

using namespace std;
using namespace stride;

namespace Tests {

using uint = unsigned int;

TEST(UnitTests__InfluenceTests, constructor) {
	Influence influence = Influence(5, 100, 5.0);
	EXPECT_EQ(influence.getInfluence(), 5.0);
	EXPECT_EQ(influence.getScore(), 0);

	Influence influence2 = Influence(0, 20, 4.0);
	EXPECT_EQ(influence2.getInfluence(), 4.0);
	EXPECT_EQ(influence.getScore(), 0);

	EXPECT_THROW(Influence(5, 100, 0.0), runtime_error);
	EXPECT_THROW(Influence(5, 100, -0.1), runtime_error);
}

TEST(UnitTests__InfluenceTests, addRecord) {
	// Also tests getScore and getInfluence
	Influence influence = Influence(7, 50, 5.0);

	uint current_score = 0;
	for (uint i = 1; i <= 7; ++i) {
		influence.addRecord(i);
		current_score += i;

		EXPECT_EQ(influence.getScore(), current_score);
		EXPECT_EQ(influence.getInfluence(), max(log10(current_score) * 50, 5.0));
	}

	influence.addRecord(10);
	EXPECT_EQ(influence.getScore(), 37);
	EXPECT_EQ(influence.getInfluence(), max(log10(37) * 50, 5.0));

	influence.addRecord(20);
	EXPECT_EQ(influence.getScore(), 55);
	EXPECT_EQ(influence.getInfluence(), max(log10(55) * 50, 5.0));

	Influence influence2 = Influence(0, 1000, 5.0);
	EXPECT_EQ(influence2.getScore(), 0);
	EXPECT_EQ(influence2.getInfluence(), 5.0);

	for (uint i = 1; i < 1000; ++i) {
		influence2.addRecord(i);
		EXPECT_EQ(influence2.getScore(), 0);
		EXPECT_EQ(influence2.getInfluence(), 5.0);
	}
}

TEST(UnitTests__InfluenceTests, addToFront) {
	Influence influence = Influence(7, 50, 5.0);

	influence.addRecord(5);
	influence.addToFront(7);

	for (uint i = 0; i < 6; ++i) {
		influence.addRecord(0);
		EXPECT_EQ(influence.getScore(), 12);
	}

	influence.addRecord(0);
	EXPECT_EQ(influence.getScore(), 0);
}

}
