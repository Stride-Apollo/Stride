#include "PopulationGenerator.h"
#include "AgeDistribution.h"
#include "FamilyParser.h"

#include <stdexcept>
#include <boost/property_tree/xml_parser.hpp>
#include <limits>
#include <algorithm>

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
	cout << "after hh\n";
	makeCities();
	cout << "after cit\n";
	makeVillages();
	cout << "after vill\n";
	placeHouseholds();
	cout << "after hhplace\n";
	makeSchools();
	cout << "after school\n";
	makeUniversities();
	cout << "after univ\n";
	makeWork();
	cout << "after comm\n";
	makeCommunities();
	assignToSchools();
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
		if ((city_dist(m_rng) == 0 && city_fractions.size() != 0) || (village_fractions.size() == 0 && city_fractions.size() != 0)) {
			uint city_index = city_dist(m_rng);
			SimpleCity& city = m_cities.at(city_index);
			city.m_current_size += household.m_indices.size();
			for (uint& person_index: household.m_indices) {
				m_people.at(person_index).m_coord = city.m_coord;
			}
		} else if (village_fractions.size() != 0) {
			uint village_index = village_dist(m_rng);
			SimpleCluster& village = m_villages.at(village_index);
			village.m_current_size += household.m_indices.size();
			for (uint& person_index: household.m_indices) {
				m_people.at(person_index).m_coord = village.m_coord;
			}
		}
	}
}

void PopulationGenerator::placeClusters(uint size, uint min_age, uint max_age, double fraction, vector<SimpleCluster>& clusters) {
	uint people = 0;

	if (min_age == 0 && max_age == 0) {
		people = m_people.size();
	} else {
		for (uint age = min_age; age <= max_age; age++) {
			people += m_age_distribution[age];
		}

	}

	uint needed_clusters = people / size + 1;
	uint city_village_size = getCityPopulation() + getVillagePopulation();

	vector<double> fractions;
	for (const SimpleCity& city: m_cities) {
		fractions.push_back(double(city.m_max_size) / double(city_village_size));
	}

	for (const SimpleCluster& village: m_villages) {
		fractions.push_back(double(village.m_max_size) / double(city_village_size));
	}

	AliasDistribution dist {fractions};
	for (uint i = 0; i < needed_clusters; i++) {
		uint village_city_index = dist(m_rng);

		if (village_city_index < m_cities.size()) {
			/// Add to a city
			SimpleCluster new_cluster;
			new_cluster.m_max_size = size;
			new_cluster.m_coord = m_cities.at(village_city_index).m_coord;
			new_cluster.m_id = m_next_id;
			m_next_id++;
			clusters.push_back(new_cluster);
		} else {
			/// Add to a village
			SimpleCluster new_cluster;
			new_cluster.m_max_size = size;
			new_cluster.m_coord = m_villages.at(village_city_index - m_cities.size()).m_coord;
			new_cluster.m_id = m_next_id;
			m_next_id++;
			clusters.push_back(new_cluster);
		}
	}
}

void PopulationGenerator::makeSchools() {
	/// Note: schools are "assigned" to villages and cities
	auto education_config = m_props.get_child("POPULATION.EDUCATION");
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.MANDATORY");
	uint school_size = education_config.get<uint>("MANDATORY.<xmlattr>.total_size");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");

	placeClusters(school_size, min_age, max_age, 1.0, m_optional_schools);
}

void PopulationGenerator::makeUniversities() {
	/// TODO check for overlap between mandatory and optional education
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE.YOUNG_EMPLOYEE");
	auto university_config = m_props.get_child("POPULATION.EDUCATION.OPTIONAL");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	double fraction = 1.0 - school_work_config.get<double>("<xmlattr>.fraction") / 100.0;
	uint size = university_config.get<uint>("<xmlattr>.total_size");

	uint intellectual_pop = 0;
	for (uint i = min_age; i <= max_age; i++) {
		intellectual_pop += m_age_distribution[i];
	}

	intellectual_pop = intellectual_pop * fraction + 1;

	auto compare_city_size = [](const SimpleCity& a, const SimpleCity b) { return a.m_max_size > b.m_max_size; };
	sort (m_cities.begin(), m_cities.end(), compare_city_size);

	uint needed_universities = intellectual_pop / size + 1;
	uint placed_universities = 0;

	while (needed_universities < placed_universities) {
		SimpleCluster univ;
		univ.m_id = m_next_id;
		univ.m_max_size = size;
		univ.m_coord = m_cities.at(placed_universities % m_cities.size()).m_coord;
		m_next_id++;
		m_mandatory_schools.push_back(univ);
		placed_universities++;
	}
}

void PopulationGenerator::makeWork() {
	/// TODO check consistency with working students and stuff
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE.EMPLOYEE");
	auto work_config = m_props.get_child("POPULATION.WORK");

	uint size = work_config.get<uint>("<xmlattr>.size");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	double fraction = school_work_config.get<double>("<xmlattr>.fraction") / 100.0;

	placeClusters(size, min_age, max_age, fraction, m_workplaces);
}

void PopulationGenerator::makeCommunities() {
	/// TODO? Currently not doing the thing with the average communities per person, right now, everyone gets two communities
	auto community_config = m_props.get_child("POPULATION.COMMUNITY");
	uint size = community_config.get<uint>("<xmlattr>.size");

	placeClusters(size, 0, 0, 1.0, m_primary_communities);
	placeClusters(size, 0, 0, 1.0, m_secondary_communities);
}

vector<uint> PopulationGenerator::getClusters(GeoCoordinate coord, double radius, const vector<SimpleCluster> clusters) const {
	vector<uint> result;
	const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
	for (uint i = 0; i < clusters.size(); i++) {
		if (calc.getDistance(coord, clusters.at(i).m_coord) <= radius) {
			result.push_back(i);
		}
	}
	return result;
}

void PopulationGenerator::assignToSchools() {
	/// TODO add factor to xml?
	auto education_config = m_props.get_child("POPULATION.EDUCATION.MANDATORY");
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.MANDATORY");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	uint start_radius = education_config.get<uint>("<xmlattr>.radius");

	double factor = 2.0;

	for (SimplePerson& person: m_people) {
		if (person.m_age >= min_age && person.m_age <= max_age) {
			vector<uint> closest_clusters_indices;

			while (closest_clusters_indices.size() == 0 && m_optional_schools.size() != 0) {
				closest_clusters_indices = getClusters(person.m_coord, start_radius, m_optional_schools);
				start_radius *= factor;
			}

			AliasDistribution cluster_dist {vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size()))};
			uint index = closest_clusters_indices.at(cluster_dist(m_rng));
			m_optional_schools.at(index).m_current_size++;
			person.m_school_id = m_optional_schools.at(index).m_id;
		}
	}
}