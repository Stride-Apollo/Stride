#include "PopulationGenerator.h"
#include "AgeDistribution.h"
#include "FamilyParser.h"

#include <stdexcept>
#include <boost/property_tree/xml_parser.hpp>

using namespace stride;
using namespace popgen;
using namespace util;
using namespace boost::property_tree;
using namespace xml_parser;

PopulationGenerator::PopulationGenerator(const string& filename) {
	try {
		read_xml(filename, m_props, trim_whitespace | no_comments);
	} catch (exception& e) {
		throw invalid_argument("Invalid file");
	}

	try {
		long int tot = m_props.get<long int>("POPULATION.<xmlattr>.total");

		if (tot < 0) {
			throw invalid_argument("Invalid attribute POPULATION::total");
		}

		m_total = tot;
	} catch(invalid_argument& e) {
		throw e;
	} catch(exception& e) {
		throw invalid_argument("Missing/invalid attribute POPULATION::total");
	}

	m_next_id = 1;
	makeRNG();
}

void PopulationGenerator::generate() {
	makeHouseholds();
	makeCities();
	// makeVillages();
	// placeHouseholds();
	// makeSchools();
	// makeUniversities();
	// makeWork();
	// makeCommunities();
	// assignToSchools();
	// assignToUniversities();
	// assignToWork();
	// assignToCommunities();
}

void PopulationGenerator::makeRNG() {
	ptree rng_config;
	long int seed = 0;
	string generator_type;

	try {
		rng_config = m_props.get_child("POPULATION.RANDOM");
		seed = rng_config.get<long int>("<xmlattr>.seed");

		if (seed < 0) {
			throw invalid_argument("Invalid attribute: POPULATION.RANDOM::seed");
		}

		generator_type = rng_config.get<string>("<xmlattr>.generator");
	} catch(invalid_argument& e) {
		throw e;
	} catch(exception& e) {
		throw invalid_argument("Missing/invalid element in POPULATION.RANDOM");
	}

	try {
		m_rng.set(generator_type, seed);
	} catch(invalid_argument& e) {
		throw e;
	}
}

void PopulationGenerator::makeHouseholds() {
	string file_name = m_props.get<string>("POPULATION.FAMILY.<xmlattr>.file");

	FamilyParser parser;
	vector<FamilyConfig> family_config {parser.parseFamilies(file_name)};

	uint current_generated = 0;

	AliasDistribution dist {vector<double>(family_config.size(), 1.0 / family_config.size())};
	while (current_generated < m_total) {
		uint family_index = dist(m_rng);
		FamilyConfig& new_config = family_config.at(family_index);

		SimpleHousehold new_household;
		new_household.m_id = m_next_id;
		for (uint& age: new_config) {
			SimplePerson new_person {age, m_next_id};
			m_people.push_back(new_person);
			new_household.m_indices.push_back(m_people.size() - 1);

			/// For visualisation purposes
			m_age_distribution[age] = m_age_distribution[age] + 1;
		}

		/// For visualisation purposes
		m_household_size[new_config.size()] = m_household_size[new_config.size()] + 1;

		m_households.push_back(new_household);
		m_next_id++;
		current_generated += new_config.size();
	}
}

void PopulationGenerator::makeCities() {
	auto cities_config = m_props.get_child("POPULATION.CITIES");
	uint size_check = 0;

	for (auto it = cities_config.begin(); it != cities_config.end(); it++) {
		if (it->first == "CITY") {
			string name = it->second.get<string>("<xmlattr>.name");
			int size = it->second.get<int>("<xmlattr>.pop");
			double latitude = it->second.get<double>("<xmlattr>.lat");
			double longitude = it->second.get<double>("<xmlattr>.lon");
			size_check += size;

			/// TODO check for errors in the 4 above declared vars

			SimpleCity new_city;
			new_city.m_max_size = size;
			new_city.m_id = m_next_id;
			m_next_id++;
			new_city.coord.m_longitude = longitude;
			new_city.coord.m_latitude = latitude;

			m_cities.push_back(new_city);
		} else {
			/// TODO exception
		}
	}
}