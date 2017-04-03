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

// TODO: make this clean, but for now, this will do (messy tests are better than no tests)

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

	if (csv.at(1).at(1) == "Brussels") {
		// The city names must be equal
		EXPECT_EQ(csv.at(1).at(1), "Brussels");
		EXPECT_EQ(csv.at(2).at(1), "Antwerp");

		// The coordinates must be the same
		EXPECT_EQ(csv.at(1).at(6), "54.567");
		EXPECT_EQ(csv.at(1).at(7), "5.89");
		EXPECT_EQ(csv.at(2).at(6), "51.123");
		EXPECT_EQ(csv.at(2).at(7), "4.567");
	} else {
		// The city names must be equal
		EXPECT_EQ(csv.at(2).at(1), "Brussels");
		EXPECT_EQ(csv.at(1).at(1), "Antwerp");

		// The coordinates must be the same
		EXPECT_EQ(csv.at(2).at(6), "54.567");
		EXPECT_EQ(csv.at(2).at(7), "5.89");
		EXPECT_EQ(csv.at(1).at(6), "51.123");
		EXPECT_EQ(csv.at(1).at(7), "4.567");
	}

	double pop1 = stod(csv.at(1).at(3));
	double pop2 = stod(csv.at(2).at(3));

	EXPECT_LT(1.0 - pop1 - pop2, 0.0001);

	// TODO not tested: is the size ok? how do we even test this? what is x_coord and y_coord? provinces? id?
}

void checkHappyDayPop (const string& file, const string& household_file) {
	vector<vector<string> > csv = readCSV(file);

	// TODO make this an argument
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
	}

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
		}
	}

	// Test household consistency
	FamilyParser parser;
	vector<FamilyConfig> family_config = parser.parseFamilies(household_file);
	map<FamilyConfig, uint> households;
	for (uint i = id.min; i <= id.max; i++) {
		FamilyConfig household;

		for (SimplePerson& person: people[i]) {
			household.push_back(person.m_age);
		}

		// Should this family configuration exist?
		EXPECT_NE(find (family_config.begin(), family_config.end(), household), family_config.end());
	}

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


	// TODO not tested: fair distribution of everything, what to do with schools?, unemployment
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
	try {
		PopulationGenerator gen {g_happy_day_file, false};
		gen.generate("cities.csv", "pop.csv", "hh.csv");

		vector<vector<string> > csv = readCSV("cities.csv");
		checkHappyDayCities(csv);

		checkHappyDayPop("pop.csv", "households_flanders.txt");

		checkHappyDayHouseHolds("hh.csv", "pop.csv");
	} catch(...) {}

}

TEST_F(PopulationGeneratorDemos, UnhappyDay) {
	

}

} //end-of-namespace-Tests
