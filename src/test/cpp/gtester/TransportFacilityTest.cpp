#include <string>
#include <vector>
#include <utility>
#include <fstream>

#include <gtest/gtest.h>
#include <boost/filesystem/operations.hpp>

#include "util/TransportFacilityReader.h"
#include "util/InstallDirs.h"

using namespace std;
using namespace stride;
using namespace util;
using namespace boost::filesystem;

TEST(TransportFacilityTest, happy_day_default) {
	const auto file_path = InstallDirs::getDataDir() /= string("transportation_facilities.csv");

	std::ifstream my_file;
	my_file.open(file_path.string());

	if (my_file.bad()) {
		throw runtime_error(string(__func__)
							+ ">Config file " + file_path.string() + " not present. Aborting.");
	}
	my_file.close();

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