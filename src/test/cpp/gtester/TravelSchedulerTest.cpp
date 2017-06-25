#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>

#include <gtest/gtest.h>
#include <boost/filesystem/operations.hpp>

#include "util/InstallDirs.h"
#include "util/TravellerScheduleReader.h"

using namespace std;
using namespace stride;
using namespace util;
using namespace boost::filesystem;
using boost::property_tree::ptree;

TEST(UnitTests__TravelSchedulerTest, happy_day) {
	const auto file_path = boost::filesystem::path(string("traveller_schedule_new_config2.xml"));

	TravellerScheduleReader reader;
	Schedule schedule = reader.readSchedule(file_path.string());

	// Loop over all days
	for (uint i = 0; i < 7; ++i) {
		Flight flight = Flight("Simulator" + to_string(i), "Simulator" + to_string(i + 1), 10 * i + 10, 20 * i + 20, i, "Antwerp", "ANR");
		EXPECT_EQ(flight, schedule[i].at(0));
		if (i != 0) {
			EXPECT_EQ(schedule[i].size(), 1U);
		} else {
			EXPECT_EQ(schedule[i].size(), 2U);
		}
	}

	// Make sure that there can be multiple flights in one day
	ASSERT_NO_THROW(schedule[0].at(1));
	EXPECT_EQ(Flight("Simulator" + to_string(0), "Simulator" + to_string(1), 80, 160, 0, "Antwerp", "ANR"), schedule[0].at(1));
}

// TODO exception tests