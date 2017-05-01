#include <string>
#include <vector>
#include <utility>

#include <gtest/gtest.h>
#include <boost/filesystem/operations.hpp>

#include "util/TransportFacilityReader.h"

using namespace std;
using namespace stride;
using namespace util;
using namespace boost::filesystem;

TEST(TransportFacilityTest, happy_day_default) {
	const auto file_path = canonical(system_complete("../data/transportation_facilities.csv"));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ ">Schedule file " + file_path.string() + " not present. Aborting.");
	}

	TransportFacilityReader reader;
	auto facilities = reader.readFacilities(file_path.string());
	EXPECT_EQ(facilities.size(), 3U);

	vector<pair<string, string> > solutions = {make_pair("Antwerp", "ANR"),
												make_pair("Antwerp", "ANR2"),
												make_pair("Brussels", "BRU")};

	for (auto& facility: solutions) {
		EXPECT_NE(find(facilities.begin(), facilities.end(), facility), facilities.end());
	}
}