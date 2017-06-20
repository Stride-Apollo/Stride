/**
 * @file
 * Implementation of tests for the Influence class.
 */

#include <gtest/gtest.h>

#include <cmath>

#include "core/Influence.h"

using namespace std;
using namespace stride;

namespace Tests {

using uint = unsigned int;

TEST(UnitTests__InfluenceTests, constructor) {
	Influence influence = Influence(5, 100);
	EXPECT_EQ(influence.getInfluence(), 0);
	EXPECT_EQ(influence.getScore(), 0);

	Influence influence2 = Influence(0, 20);
	EXPECT_EQ(influence2.getInfluence(), 0);
	EXPECT_EQ(influence.getScore(), 0);
}

TEST(UnitTests__InfluenceTests, addRecord) {
	// Also tests getScore and getInfluence
	Influence influence = Influence(7, 50);

	uint current_score = 0;
	for (uint i = 1; i <= 7; ++i) {
		influence.addRecord(i);
		current_score += i;

		EXPECT_EQ(influence.getScore(), current_score);
		EXPECT_EQ(influence.getInfluence(), log10(current_score) * 50);
	}

	influence.addRecord(10);
	EXPECT_EQ(influence.getScore(), 37);
	EXPECT_EQ(influence.getInfluence(), log10(37) * 50);

	influence.addRecord(20);
	EXPECT_EQ(influence.getScore(), 55);
	EXPECT_EQ(influence.getInfluence(), log10(55) * 50);

	Influence influence2 = Influence(0, 1000);
	EXPECT_EQ(influence2.getScore(), 0);
	EXPECT_EQ(influence2.getInfluence(), 0);

	for (uint i = 1; i < 1000; ++i) {
		influence2.addRecord(i);
		EXPECT_EQ(influence2.getScore(), 0);
		EXPECT_EQ(influence2.getInfluence(), 0);
	}
}

}
