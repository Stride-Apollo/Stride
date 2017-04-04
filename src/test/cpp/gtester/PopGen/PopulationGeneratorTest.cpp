/**
 * @file
 * Implementation of tests for the Population Generator.
 */

#include "popgen/PopulationGenerator.h"
#include "popgen/FamilyParser.h"
#include "util/StringUtils.h"
#include "util/InstallDirs.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <boost/tokenizer.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <limits>
#include <algorithm>

using namespace std;
using namespace stride;
using namespace popgen;
using namespace ::testing;
using namespace boost;
using namespace util;

namespace Tests {

class PopulationGeneratorDemos: public ::testing::Test {
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~PopulationGeneratorDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
	static const string              g_happy_day_file;
	static const string              g_no_cities_file;
	static const string              g_no_villages_file;
	static const string              g_invalid_syntax_file;
	static const string              g_input_violation_file;
	static const string              g_contradiction_file;
	static const string              g_non_existent_file;
	static const string              g_output_prefix;
};

// Default values
const string            PopulationGeneratorDemos::g_happy_day_file            = "happy_day.xml";
const string            PopulationGeneratorDemos::g_no_cities_file            = "no_cities.xml";
const string            PopulationGeneratorDemos::g_no_villages_file          = "no_villages.xml";
const string            PopulationGeneratorDemos::g_invalid_syntax_file       = "invalid_syntax.xml";
const string            PopulationGeneratorDemos::g_input_violation_file      = "input_violation.xml";
const string            PopulationGeneratorDemos::g_contradiction_file        = "contradiction.xml";
const string            PopulationGeneratorDemos::g_non_existent_file         = "non_existent.xml";
const string            PopulationGeneratorDemos::g_output_prefix             = "PopulationGenerator";

// TODO:
	// sizes of cities/villages
	// fair distribution of everything (unemployment, sizes, families,...)
	// More exception tests (like non-existent household file!)

vector<vector<string> > readCSV(string file) {
	ifstream my_file((InstallDirs::getDataDir() /= file).string());

	if (!my_file.is_open()) {
		return vector<vector<string> >();
	}

	using Tokenizer = tokenizer< escaped_list_separator<char> >;

	vector<vector<string> > result;

	string line;

	while (getline(my_file,line)) {
		Tokenizer tok(line);

		vector<string> tmp;
		tmp.assign(tok.begin(),tok.end());
		result.push_back(tmp);
	}

	return result;
}

void checkHappyDayCities(const vector<vector<string> >& csv) {
	// Note: this function is specifically for a certain file
	// The headers must be equal
	vector<string> header {"city_id", "city_name", "province", "population", "x_coord", "y_coord", "latitude", "longitude"};
	EXPECT_EQ(csv.at(0), header);

	set<string> unique_column;
	vector<GeoCoordinate> unique_coord;
	/// Go over every column (except the latitude-longitude one cause they belong to each other)
	for (uint i = 0; i < 7; i++) {
		unique_coord.clear();
		unique_column.clear();
		for (uint j = 1; j < csv.size(); j++) {
			const vector<string>& row = csv.at(j);
			if (i == 2) {
				EXPECT_TRUE(stoi(row.at(i)) <= 10 && stoi(row.at(i)) >= 1);
			} else if (i >= 3 && i <= 5) {
				break;
			} else if (i == 6) {
				/// Test for unique coordinates
				double lat = stod(row.at(i));
				double lon = stod(row.at(i + 1));

				EXPECT_EQ(find(unique_coord.begin(), unique_coord.end(), GeoCoordinate(lat, lon)), unique_coord.end());

				unique_coord.push_back(GeoCoordinate(lat, lon));
			} else {
				/// When here, we're talking about city id or name, 
				uint previous_size = unique_column.size();
				unique_column.insert(row.at(i));
				EXPECT_EQ(previous_size, unique_column.size() - 1);
			}
		}
	}

	double pop = 0.0;
	for (uint i = 1; i < csv.size(); i++) {
		const vector<string>& row = csv.at(i);
		pop += stod(row.at(3));
	}

	EXPECT_NEAR(1.0, pop, 0.0001);
}

void checkHappyDayPop (const string& file, const string& household_file) {
	vector<vector<string> > csv = readCSV(file);

	uint work_max_size = 20;

	// The headers must be equal
	vector<string> header {"age","household_id","school_id","work_id","primary_community","secondary_community"};
	EXPECT_EQ(csv.at(0), header);

	// Load the people in a map (key = household_id)
	map<uint, vector<SimplePerson> > people;
	MinMax id {StringUtils::fromString<unsigned int>(csv.at(1)[1]), 0};
	for (uint i = 1; i < csv.size(); i++) {
		const vector<string>& current_person = csv.at(i);

		SimplePerson person;
		person.m_age = StringUtils::fromString<unsigned int>(current_person[0]);
		person.m_household_id = StringUtils::fromString<unsigned int>(current_person[1]);
		person.m_school_id = StringUtils::fromString<unsigned int>(current_person[2]);
		person.m_work_id = StringUtils::fromString<unsigned int>(current_person[3]);
		person.m_primary_community = StringUtils::fromString<unsigned int>(current_person[4]);
		person.m_secondary_community = StringUtils::fromString<unsigned int>(current_person[5]);

		people[person.m_household_id].push_back(person);

		if (person.m_household_id > id.max) {
			id.max = person.m_household_id;
		}

		if (person.m_household_id < id.min) {
			id.min = person.m_household_id;
		}

		/// Test on the ages: e.g. a 50 year old can't go to mandatory schools
		if (person.m_age >= 26) {
			EXPECT_EQ(person.m_school_id, 0U);
		} else if (person.m_age <= 25 && person.m_age >= 18) {
			/// The person could either work or go to school, but not both
			EXPECT_TRUE((
				person.m_school_id != 0U && person.m_work_id == 0U) ||
				(person.m_school_id == 0U && person.m_work_id != 0U) ||
				(person.m_school_id == 0U && person.m_work_id == 0U));
		} else if (person.m_age <= 17 && person.m_age >= 3) {
			/// Little children have to go to school
			EXPECT_NE(person.m_school_id, 0U);
		} else {
			/// Babies / those little people that run around the house
			EXPECT_EQ(person.m_school_id, 0U);
		}
	}

	// Test household consistency
	FamilyParser parser;
	vector<FamilyConfig> family_config = parser.parseFamilies(household_file);
	map<FamilyConfig, uint> households;
	uint biggest_family = 0;
	for (uint i = id.min; i <= id.max; i++) {
		FamilyConfig household;

		for (SimplePerson& person: people[i]) {
			household.push_back(person.m_age);
		}

		if (household.size() > biggest_family) {
			biggest_family = household.size();
		}

		// Should this family configuration exist?
		EXPECT_NE(find (family_config.begin(), family_config.end(), household), family_config.end());
	}

	map<uint, uint> community;
	// Test for communities within each household
	for (uint i = id.min; i <= id.max; i++) {
		uint primary_community = 0;
		uint secondary_community = 0;
		if (people[i].size() > 0) {
			primary_community = people[i].at(0).m_primary_community;
			secondary_community = people[i].at(0).m_secondary_community;
		}

		for (SimplePerson& person: people[i]) {
			EXPECT_EQ(person.m_primary_community, primary_community);
			EXPECT_EQ(person.m_secondary_community, secondary_community);
			community[person.m_primary_community]++;
			community[person.m_secondary_community]++;
			EXPECT_LT(community[person.m_primary_community], 2000U + biggest_family);
			EXPECT_LT(community[person.m_secondary_community], 2000U + biggest_family);
		}
	}

	EXPECT_EQ(community[0], 0U);

	// Check work ID consistency
	map<uint, uint> work;
	for (uint i = id.min; i <= id.max; i++) {
		for (SimplePerson& person: people[i]) {
			if (person.m_work_id != 0) {
				work[person.m_work_id]++;

				EXPECT_LT(work[person.m_work_id], work_max_size + 1);
			}
		}
	}
}

void checkHappyDayHouseHolds(const string& household_file, const string& pop_file) {
	// Load the households in a map (id, size)
	vector<vector<string> > csv = readCSV(household_file);

	// The headers must be equal
	vector<string> header {"hh_id","latitude","longitude","size"};
	EXPECT_EQ(csv.at(0), header);

	map<uint, uint> household_id_size;
	for (uint i = 1; i < csv.size(); i++) {
		uint id = StringUtils::fromString<unsigned int>(csv.at(i)[0]);
		uint size = StringUtils::fromString<unsigned int>(csv.at(i)[3]);
		household_id_size[id] = size;
	}



	// Load the people in a map (key = household_id)
	csv = readCSV(pop_file);
	map<uint, vector<SimplePerson> > people;
	MinMax id {StringUtils::fromString<unsigned int>(csv.at(1)[1]), 0};
	for (uint i = 1; i < csv.size(); i++) {
		const vector<string>& current_person = csv.at(i);

		SimplePerson person;
		person.m_age = StringUtils::fromString<unsigned int>(current_person[0]);
		person.m_household_id = StringUtils::fromString<unsigned int>(current_person[1]);
		person.m_school_id = StringUtils::fromString<unsigned int>(current_person[2]);
		person.m_work_id = StringUtils::fromString<unsigned int>(current_person[3]);
		person.m_primary_community = StringUtils::fromString<unsigned int>(current_person[4]);
		person.m_secondary_community = StringUtils::fromString<unsigned int>(current_person[5]);

		people[person.m_household_id].push_back(person);

		if (person.m_household_id > id.max) {
			id.max = person.m_household_id;
		}

		if (person.m_household_id < id.min) {
			id.min = person.m_household_id;
		}
	}

	for (uint i = id.min; i <= id.max; i++) {
		EXPECT_EQ(people[i].size(), household_id_size[i]);
	}
}


TEST_F(PopulationGeneratorDemos, HappyDay_default) {
	// Tests which reflect the regular use

	// -----------------------------------------------------------------------------------------
	// Actual tests
	// -----------------------------------------------------------------------------------------
	PopulationGenerator gen {g_happy_day_file, false};
	gen.generate("cities.csv", "pop.csv", "hh.csv");

	vector<vector<string> > csv = readCSV("cities.csv");
	checkHappyDayCities(csv);

	checkHappyDayPop("pop.csv", "households_flanders.txt");

	checkHappyDayHouseHolds("hh.csv", "pop.csv");

}

TEST_F(PopulationGeneratorDemos, UnhappyDay_default) {
	EXPECT_THROW(PopulationGenerator(g_no_cities_file, false), invalid_argument);
	EXPECT_THROW(PopulationGenerator(g_no_villages_file, false), invalid_argument);
	EXPECT_THROW(PopulationGenerator(g_invalid_syntax_file, false), invalid_argument);
	EXPECT_THROW(PopulationGenerator(g_input_violation_file, false), invalid_argument);
	EXPECT_THROW(PopulationGenerator(g_contradiction_file, false), invalid_argument);
	EXPECT_THROW(PopulationGenerator(g_non_existent_file, false), invalid_argument);
	EXPECT_THROW(PopulationGenerator(g_output_prefix, false), invalid_argument);
}

} //end-of-namespace-Tests