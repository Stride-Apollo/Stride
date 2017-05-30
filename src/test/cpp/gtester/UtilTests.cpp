
#include <iostream>

#include <gtest/gtest.h>

#include "util/SimplePlanner.h"

using namespace std;
using namespace stride;
using namespace util;
using namespace ::testing;

namespace Tests {

using SI = SimplePlanner<int>;

TEST(UnitTests__Utils, SimplePlanner) {
	SI planner;
	planner.add(0, 0);
	planner.add(0, 1);
	planner.add(1, 10);
	planner.add(1, 11);
	planner.add(5, 50);
	planner.getModifiableDay(5)->emplace_back(new int(51));

	planner.nextDay();
	EXPECT_EQ(*(planner.today()->at(0)), 10);
	EXPECT_EQ(*(planner.today()->at(1)), 11);

	EXPECT_EQ(planner.getDay(6)->size(), 0);

	EXPECT_EQ(*(planner.getDay(4)->at(0)), 50);
	EXPECT_EQ(*(planner.getDay(4)->at(1)), 51);

	planner.nextDay();
	planner.nextDay();
	planner.nextDay();

	EXPECT_EQ(*(planner.getDay(1)->at(0)), 50);
	EXPECT_EQ(*(planner.getDay(1)->at(1)), 51);

	planner.nextDay();
	planner.nextDay();
	planner.nextDay();

	EXPECT_EQ(planner.getDay(0)->size(), 0);
	EXPECT_EQ(planner.getDay(1345)->size(), 0);
}

}
