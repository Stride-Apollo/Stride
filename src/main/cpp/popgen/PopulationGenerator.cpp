#include "PopulationGenerator.h"
#include "AgeDistribution.h"
#include "FamilyParser.h"

#include <stdexcept>
#include <boost/property_tree/xml_parser.hpp>
#include <limits>

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
	makeVillages();
	placeHouseholds();
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
			new_person.m_household_id = m_next_id;
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
			new_city.m_current_size = 0;
			new_city.m_id = m_next_id;
			m_next_id++;
			new_city.m_coord.m_longitude = longitude;
			new_city.m_coord.m_latitude = latitude;

			m_cities.push_back(new_city);
		} else {
			/// TODO exception
		}
	}
}

GeoCoordinate PopulationGenerator::getCityMiddle() const {
	double latitude_middle = 0.0;
	double longitude_middle = 0.0;
	for (const SimpleCity& city: m_cities) {
		latitude_middle += city.m_coord.m_latitude;
		longitude_middle += city.m_coord.m_longitude;
	}
	latitude_middle /= m_cities.size();
	longitude_middle /= m_cities.size();

	GeoCoordinate result;
	result.m_latitude = latitude_middle;
	result.m_longitude = longitude_middle;
	return result;
}

double PopulationGenerator::getCityRadius(const GeoCoordinate& coord) const {
	double current_maximum = -1.0;

	const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
	for (const SimpleCity& city: m_cities) {
		double distance = calc.getDistance(coord, city.m_coord);
		if (distance > current_maximum) {
			current_maximum = distance;
		}
	}

	return current_maximum;
}

double PopulationGenerator::getCityPopulation() const {
	uint result = 0;

	for (const SimpleCity& city: m_cities) {
		result += city.m_max_size;
	}

	return result;
}

double PopulationGenerator::getVillagePopulation() const {
	uint result = 0;

	for (const SimpleCluster& village: m_villages) {
		result += village.m_max_size;
	}

	return result;
}

void PopulationGenerator::makeVillages() {
	auto village_config = m_props.get_child("POPULATION.VILLAGES");
	int village_radius_factor = village_config.get<double>("<xmlattr>.radius");
	GeoCoordinate middle = getCityMiddle();
	double radius = getCityRadius(middle);
	uint city_population = getCityPopulation();
	int unassigned_population = m_people.size() - city_population;

	vector<double> fractions;
	vector<MinMax> boundaries;
	for (auto it = village_config.begin(); it != village_config.end(); it++) {
		if (it->first == "VILLAGE") {
			uint min = it->second.get<uint>("<xmlattr>.min");
			uint max = it->second.get<uint>("<xmlattr>.max");
			double fraction = it->second.get<double>("<xmlattr>.fraction") / 100.0;
			MinMax min_max {min, max};

			fractions.push_back(fraction);
			boundaries.push_back(min_max);
		}
	}

	AliasDistribution village_type_dist {fractions};
	const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
	while (unassigned_population > 0) {
		/// TODO make sure generated coordinates are unique!
		uint village_type_index = village_type_dist(m_rng);
		MinMax village_pop = boundaries.at(village_type_index);
		uint range_interval_size = village_pop.max - village_pop.min + 1;

		AliasDistribution village_size_dist {vector<double>(range_interval_size, 1.0 / range_interval_size)};
		uint village_size = village_size_dist(m_rng) + village_pop.min;

		SimpleCluster new_village;
		new_village.m_max_size = village_size;
		new_village.m_id = m_next_id;
		m_next_id++;
		new_village.m_coord = calc.generateRandomCoord(middle, radius * village_radius_factor, m_rng);
		m_villages.push_back(new_village);
		unassigned_population -= new_village.m_max_size;
	}
}

void PopulationGenerator::placeHouseholds() {
	uint city_pop = getCityPopulation();
	uint village_pop = getVillagePopulation();
	uint total_pop = village_pop + city_pop;	/// Note that this number may slightly differ from other "total pop" numbers

	vector<double> village_fractions;
	for (SimpleCluster& village: m_villages) {
		village_fractions.push_back(double(village.m_max_size) / double(village_pop));
	}

	vector<double> city_fractions;
	for (SimpleCity& city: m_cities) {
		city_fractions.push_back(double(city.m_max_size) / double(city_pop));
	}

	AliasDistribution village_city_dist { {double(city_pop) / double(total_pop), double(village_pop) / double(total_pop)} };
	AliasDistribution city_dist {city_fractions};
	AliasDistribution village_dist {village_fractions};
	for (SimpleHousehold& household: m_households) {
		if (city_dist(m_rng) == 0) {
			uint city_index = city_dist(m_rng);
			SimpleCity& city = m_cities.at(city_index);
			city.m_current_size += household.m_indices.size();
			for (uint& person_index: household.m_indices) {
				m_people.at(person_index).m_coord = city.m_coord;
			}
		} else {
			uint village_index = village_dist(m_rng);
			SimpleCluster& village = m_villages.at(village_index);
			village.m_current_size += household.m_indices.size();
			for (uint& person_index: household.m_indices) {
				m_people.at(person_index).m_coord = village.m_coord;
			}
		}
	}
}