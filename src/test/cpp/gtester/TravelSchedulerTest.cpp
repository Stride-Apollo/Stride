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

TEST(TravelSchedulerTest, happy_day_default) {
	const auto file_path = InstallDirs::getDataDir() /= string("traveller_schedule.json");

	std::ifstream my_file;
	my_file.open(file_path.string());

	if (my_file.bad()) {
		throw runtime_error(string(__func__)
							+ ">Config file " + file_path.string() + " not present. Aborting.");
	}
	my_file.close();

	TravellerScheduleReader reader;
	Schedule schedule = reader.readSchedule(file_path.string());

	// Loop over all days
	for (uint i = 0; i < 7; ++i) {
		Flight flight = Flight(i, i + 1, 10 * i + 10, 20 * i + 20, i, "Antwerp", "ANR");
		EXPECT_EQ(flight, schedule[i].at(0));

		// Check the amount of scheduled flights
		if (i != 0) {
			EXPECT_EQ(schedule[i].size(), 1U);
		} else {
			EXPECT_EQ(schedule[i].size(), 2U);
		}
	}

	// Make sure that there can be multiple flights in one day
	ASSERT_NO_THROW(schedule[0].at(1));
	EXPECT_EQ(Flight(0, 1, 80, 160, 0, "Antwerp", "ANR"), schedule[0].at(1));
}

// TODO exception tests