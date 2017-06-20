/**
 * @file
 * Implementation of tests for the District class.
 */

#include <gtest/gtest.h>

#include <cmath>

#include "core/District.h"

using namespace std;
using namespace stride;

namespace Tests {

using uint = unsigned int;

TEST(UnitTests__DistrictTests, constructor) {
	District district = District("someName", 10, 5, GeoCoordinate(50.0, 10.0));

	EXPECT_EQ(district.getName(), "someName");
	EXPECT_EQ(district.getLocation(), GeoCoordinate(50.0, 10.0));
}

TEST(UnitTests__DistrictTests, facilities) {
	District district = District("someName", 10, 10);

	EXPECT_FALSE(district.hasFacility("NOPE"));

	district.addFacility("someFacility");
	EXPECT_TRUE(district.hasFacility("someFacility"));
	
	district.addFacility("someOtherFacility");
	EXPECT_TRUE(district.hasFacility("someFacility"));
	EXPECT_TRUE(district.hasFacility("someOtherFacility"));
}

TEST(UnitTests__DistrictTests, influences) {
	District district = District("someName", 10, 10);

	district.addFacility("someFacility");
	EXPECT_EQ(district.getFacilityInfluence("someFacility"), 0);
	EXPECT_EQ(district.getFacilityInfluence("nonexistent"), 0);
	
	district.addFacility("someOtherFacility");
	district.visitFacility("someOtherFacility", 20);
	EXPECT_EQ(district.getFacilityInfluence("someFacility"), 0);
	EXPECT_GT(district.getFacilityInfluence("someOtherFacility"), 0);

	district.visitFacility("someFacility", 10);

	for (uint i = 0; i < 10; ++i) {
		EXPECT_GT(district.getFacilityInfluence("someOtherFacility"), 0);
		EXPECT_GT(district.getFacilityInfluence("someFacility"), 0);
		district.advanceInfluencesRecords();
	}

	EXPECT_EQ(district.getFacilityInfluence("someFacility"), 0);
	EXPECT_EQ(district.getFacilityInfluence("someOtherFacility"), 0);
}

}
