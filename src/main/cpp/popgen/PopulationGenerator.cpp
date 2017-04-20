#include "PopulationGenerator.h"
#include "FamilyParser.h"
#include "util/InstallDirs.h"
#include "util/TimeStamp.h"

#include <stdexcept>
#include <boost/property_tree/xml_parser.hpp>
#include <limits>
#include <algorithm>

using namespace stride;
using namespace popgen;
using namespace util;
using namespace boost::property_tree;
using namespace xml_parser;

PopulationGenerator::PopulationGenerator(const string& filename, bool output) {
	// check data environment.
	if (InstallDirs::getDataDir().empty()) {
		throw runtime_error(string(__func__) + "> Data directory not present! Aborting.");
	}

	try {
		read_xml((InstallDirs::getDataDir() /= filename).string(), m_props, trim_whitespace | no_comments);
	} catch (exception& e) {
		throw invalid_argument("Invalid file");
	}

	try {
		long int tot = m_props.get<long int>("POPULATION.<xmlattr>.total");

		if (tot <= 0) {
			throw invalid_argument("Invalid attribute POPULATION::total");
		}

		m_total = tot;
	} catch(invalid_argument& e) {
		throw e;
	} catch(exception& e) {
		throw invalid_argument("Missing/invalid attribute POPULATION::total");
	}

	m_next_id = 1;
	m_output = output;
	makeRNG();

	try {
		if (!m_output) {
			cerr.setstate(ios_base::failbit);
		}
		chechForValidXML();
		cerr.clear();
	} catch (...) {
		cerr.clear();
		throw;
	}
}

void PopulationGenerator::generate(const string& target_cities, const string& target_pop, const string& target_households, const string& target_clusters) {
	if (!m_output) {
		cerr.setstate(ios_base::failbit);
	}

	try {
		cerr << "Generating " << m_total << " people...\n";
		makeHouseholds();
		makeCities();
		makeVillages();
		placeHouseholds();
		makeSchools();
		makeUniversities();
		makeWork();
		makeCommunities();
		assignToSchools();
		assignToUniversities();
		assignToWork();
		assignToCommunities();
		cerr << "Generated " << m_people.size() << " people\n";

		writeCities(target_cities);
		writePop(target_pop);
		writeHouseholds(target_households);
		writeClusters(target_clusters);
		cerr.clear();
	} catch(...) {
		cerr.clear();
		throw;
	}
	cerr.clear();
}

void PopulationGenerator::writeCities(const string& target_cities) const {
	ofstream my_file {(InstallDirs::getDataDir() /= target_cities).string()};
	double total_pop = 0.0;

	for (const SimpleCity& city: m_cities) {
		total_pop += city.m_current_size;
	}

	for (const SimpleCluster& village: m_villages) {
		total_pop += village.m_current_size;
	}

	if (my_file.is_open()) {
		my_file << "\"city_id\",\"city_name\",\"province\",\"population\",\"x_coord\",\"y_coord\",\"latitude\",\"longitude\"\n";

		/// Picking the province would be better somewhere else, but that would require a big refactor, so I forgive myself for this one
		uint provinces = m_props.get<uint>("POPULATION.<xmlattr>.provinces");
		AliasDistribution dist { vector<double>(provinces, 1.0 / provinces) };

		for (const SimpleCity& city: m_cities) {
			my_file.precision(std::numeric_limits<double>::max_digits10);
			my_file << city.m_id
				<< ",\""
				<< city.m_name
				<< "\"," << dist(m_rng) + 1 << ","
				<< city.m_current_size / total_pop
				<< ",0,0,"
				<< city.m_coord.m_latitude
				<< ","
				<< city.m_coord.m_longitude
				<< endl;
		}

		uint village_counter = 1;
		for (const SimpleCluster& village: m_villages) {
			my_file.precision(std::numeric_limits<double>::max_digits10);
			my_file << village.m_id
				<< ",\""
				<< village_counter
				<< "\"," << dist(m_rng) + 1 << ","
				<< village.m_current_size / total_pop
				<< ",0,0,"
				<< village.m_coord.m_latitude
				<< ","
				<< village.m_coord.m_longitude
				<< endl;
			village_counter++;
		}

		my_file.close();
	} else {
		throw invalid_argument("In PopulationGenerator: Invalid file.");
	}
}

void PopulationGenerator::writePop(const string& target_pop) const {
	ofstream my_file {(InstallDirs::getDataDir() /= target_pop).string()};
	if (my_file.is_open()) {
		my_file << "\"age\",\"household_id\",\"school_id\",\"work_id\",\"primary_community\",\"secondary_community\"\n";

		for (const SimplePerson& person: m_people) {
			my_file << person.m_age << ","
				<< person.m_household_id << ","
				<< person.m_school_id << ","
				<< person.m_work_id << ","
				<< person.m_primary_community << ","
				<< person.m_secondary_community
				<< endl;
		}

		my_file.close();
	} else {
		throw invalid_argument("In PopulationGenerator: Invalid file.");
	}
}

void PopulationGenerator::writeHouseholds(const string& target_households) const {
	ofstream my_file {(InstallDirs::getDataDir() /= target_households).string()};
	if (my_file.is_open()) {
		my_file << "\"hh_id\",\"latitude\",\"longitude\",\"size\"\n";

		for (const SimpleHousehold& household: m_households) {
			my_file << household.m_id << ","
				<< m_people.at(household.m_indices.at(0)).m_coord.m_latitude << ","
				<< m_people.at(household.m_indices.at(0)).m_coord.m_longitude << ","
				<< household.m_indices.size()
				<< endl;
		}

		my_file.close();
	} else {
		throw invalid_argument("In PopulationGenerator: Invalid file.");
	}
}

void PopulationGenerator::writeClusters(const string& target_clusters) const {
	ofstream my_file {(InstallDirs::getDataDir() /= target_clusters).string()};
	if (my_file.is_open()) {
		my_file << "\"cluster_id\",\"cluster_type\",\"latitude\",\"longitude\"\n";

		vector<ClusterType> types {ClusterType::Household,
									ClusterType::School,
									ClusterType::Work,
									ClusterType::PrimaryCommunity,
									ClusterType::SecondaryCommunity,
									ClusterType::Null};

		for (auto& cluster_type: types) {
			uint current_id = 1;
			while (m_locations.find(make_pair(cluster_type, current_id)) != m_locations.end()) {
				my_file.precision(std::numeric_limits<double>::max_digits10);
				my_file << current_id << ","
					<< toString(cluster_type) << ","
					<< m_locations.at(make_pair(cluster_type, current_id)).m_latitude << ","
					<< m_locations.at(make_pair(cluster_type, current_id)).m_longitude
					<< endl;

				++current_id;
			}
		}

		my_file.close();
	} else {
		throw invalid_argument("In PopulationGenerator: Invalid file.");
	}
}

void PopulationGenerator::chechForValidXML() const {
	try {
		auto pop_config = m_props.get_child("POPULATION");

		/// RNG is already valid at this point (made in constructor)
		/// Check for FAMILY tag must be done during parsing

		/// Check for the provinces
		int provinces = pop_config.get<int>("<xmlattr>.provinces");

		if (provinces <= 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}

		/// Cities: unique location
		cerr << "\rChecking for valid XML [0%]";
		int total_size = 0;
		bool has_no_cities = true;
		auto cities_config = pop_config.get_child("CITIES");
		vector<GeoCoordinate> current_locations;
		for (auto it = cities_config.begin(); it != cities_config.end(); it++) {
			if (it->first == "CITY") {
				has_no_cities = false;
				it->second.get<string>("<xmlattr>.name");
				total_size += it->second.get<int>("<xmlattr>.pop");
				double latitude = it->second.get<double>("<xmlattr>.lat");
				double longitude = it->second.get<double>("<xmlattr>.lon");

				if (abs(latitude) > 90 || abs(longitude) > 180) {
					throw invalid_argument("In PopulationGenerator: Invalid geo-coordinate in XML.");
				}

				if (it->second.get<int>("<xmlattr>.pop") <= 0) {
					throw invalid_argument("In PopulationGenerator: Numerical error.");
				}

				auto it2 = find (current_locations.begin(), current_locations.end(), GeoCoordinate(latitude, longitude));
				if (it2 != current_locations.end())
					throw invalid_argument("In PopulationGenerator: Duplicate coordinates given in XML.");

				current_locations.push_back(GeoCoordinate(latitude, longitude));
			} else {
				throw invalid_argument("In PopulationGenerator: Missing/incorrect tags/attributes in XML.");
			}
		}

		if (has_no_cities) {
			throw invalid_argument("In PopulationGenerator: No cities found.");
		}

		// if (total_size > m_total) {
		// 	/// TODO is this necessary?
		// 	/// throw invalid_argument("In PopulationGenerator: City population in XML exceeds the total population.");
		// }


		/// Check for valid villages
		cerr << "\rChecking for valid XML [18%]";
		auto village_config = pop_config.get_child("VILLAGES");
		double village_radius_factor = village_config.get<double>("<xmlattr>.radius");

		bool has_no_villages = true;

		double fraction = 0.0;
		for (auto it = village_config.begin(); it != village_config.end(); it++) {
			if (it->first == "VILLAGE") {
				has_no_villages = false;
				int minimum = it->second.get<int>("<xmlattr>.min");
				int max = it->second.get<int>("<xmlattr>.max");
				fraction += it->second.get<double>("<xmlattr>.fraction") / 100.0;

				if (fraction < 0) {
					throw invalid_argument("In PopulationGenerator: Numerical error.");
				}

				if (minimum > max || minimum <= 0 || max < 0) {
					throw invalid_argument("In PopulationGenerator: Numerical error.");
				}
			} else if (it->first == "<xmlattr>") {
			} else {
				throw invalid_argument("In PopulationGenerator: Missing/incorrect tags/attributes in XML.");
			}
		}

		if (fraction != 1.0 || village_radius_factor <= 0.0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}

		if (has_no_villages) {
			throw invalid_argument("In PopulationGenerator: No villages found.");
		}


		/// Check for valid Education
		/// Mandatory education
		cerr << "\rChecking for valid XML [36%]";
		auto education_config = pop_config.get_child("EDUCATION");
		auto school_work_config = pop_config.get_child("SCHOOL_WORK_PROFILE");
		total_size = education_config.get<int>("MANDATORY.<xmlattr>.total_size");
		int cluster_size = education_config.get<int>("MANDATORY.<xmlattr>.cluster_size");
		int mandatory_min = school_work_config.get<int>("MANDATORY.<xmlattr>.min");
		int mandatory_max = school_work_config.get<int>("MANDATORY.<xmlattr>.max");
		double radius = education_config.get<double>("MANDATORY.<xmlattr>.radius");

		if (mandatory_min > mandatory_max || mandatory_min < 0 || mandatory_max < 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error in min/max pair.");
		}

		if (total_size <= 0 || cluster_size <= 0 || radius <= 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}

		/// Optional education
		cerr << "\rChecking for valid XML [42%]";
		school_work_config = pop_config.get_child("SCHOOL_WORK_PROFILE.EMPLOYABLE.YOUNG_EMPLOYEE");
		int minimum = school_work_config.get<int>("<xmlattr>.min");
		int max = school_work_config.get<int>("<xmlattr>.max");
		cluster_size = education_config.get<int>("OPTIONAL.<xmlattr>.cluster_size");
		fraction = 1.0 - school_work_config.get<double>("<xmlattr>.fraction") / 100.0;
		total_size = education_config.get<uint>("OPTIONAL.<xmlattr>.total_size");
		radius = education_config.get<double>("OPTIONAL.<xmlattr>.radius");

		if (minimum > max || minimum < 0 || max < 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error in min/max pair.");
		}

		if (total_size <= 0 || cluster_size <= 0 || radius <= 0 || fraction < 0.0 || fraction > 1.0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}

		if (minimum <= mandatory_max) {
			throw invalid_argument("In PopulationGenerator: Overlapping min/max pairs.");
		}

		fraction = education_config.get<double>("OPTIONAL.FAR.<xmlattr>.fraction") / 100.0;

		if (fraction < 0.0 || fraction > 1.0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}



		/// Check for valid work
		cerr << "\rChecking for valid XML [68%]";
		school_work_config = pop_config.get_child("SCHOOL_WORK_PROFILE.EMPLOYABLE");
		auto work_config = pop_config.get_child("WORK");

		total_size = work_config.get<int>("<xmlattr>.size");
		minimum = school_work_config.get<int>("EMPLOYEE.<xmlattr>.min");
		max = school_work_config.get<int>("EMPLOYEE.<xmlattr>.max");
		fraction = school_work_config.get<double>("<xmlattr>.fraction") / 100.0;
		radius = work_config.get<double>("FAR.<xmlattr>.radius");

		if (minimum > max || minimum < 0 || max < 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error in min/max pair.");
		}

		if (total_size <= 0 || cluster_size <= 0 || radius <= 0 || fraction < 0.0 || fraction > 1.0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}

		int min2 = school_work_config.get<int>("YOUNG_EMPLOYEE.<xmlattr>.min");
		int max2 = school_work_config.get<int>("YOUNG_EMPLOYEE.<xmlattr>.max");
		fraction = work_config.get<double>("FAR.<xmlattr>.fraction") / 100.0;

		if (min2 > max2 || min2 < 0 || max2 < 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error in min/max pair.");
		}

		if (max2 >= minimum) {
			throw invalid_argument("In PopulationGenerator: Overlapping min/max pairs.");
		}

		if (fraction < 0.0 || fraction > 1.0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}



		/// Check for valid communities
		cerr << "\rChecking for valid XML [84%]";
		total_size = pop_config.get<int>("COMMUNITY.<xmlattr>.size");
		radius = pop_config.get<int>("COMMUNITY.<xmlattr>.radius");

		if (total_size <= 0 || radius <= 0) {
			throw invalid_argument("In PopulationGenerator: Numerical error.");
		}
		cerr << "\rChecking for valid XML [100%]\n";



	} catch(invalid_argument& e) {
		cerr << "\n";
		throw e;
	} catch(exception& e) {
		/// Boost exceptions due to missing tags, wrong tags,...
		cerr << "\n";
		throw invalid_argument("In PopulationGenerator: Missing/incorrect tags/attributes in XML.");
	}
}

void PopulationGenerator::makeRNG() {
	ptree rng_config;
	long int seed = 0;
	string generator_type;

	try {
		rng_config = m_props.get_child("POPULATION.RANDOM");
		seed = rng_config.get<long int>("<xmlattr>.seed");

		if (seed < 0) {
			throw invalid_argument("In PopulationGenerator: Missing/incorrect tags/attributes in XML.");
		}

		generator_type = rng_config.get<string>("<xmlattr>.generator");

		/// This might throw an exception, but we'll just rethrow it
		m_rng.set(generator_type, seed);
	} catch(invalid_argument& e) {
		throw e;
	} catch(exception& e) {
		throw invalid_argument("In PopulationGenerator: Missing/incorrect tags/attributes in XML.");
	}
}

void PopulationGenerator::makeHouseholds() {
	m_next_id = 1;
	string file_name = m_props.get<string>("POPULATION.FAMILY.<xmlattr>.file");

	FamilyParser parser;

	vector<FamilyConfig> family_config {parser.parseFamilies(file_name)};

	uint current_generated = 0;

	/// Uniformly choose between the given family configurations
	AliasDistribution dist {vector<double>(family_config.size(), 1.0 / family_config.size())};
	while (current_generated < m_total) {
		cerr << "\rGenerating households [" << min(uint(double(current_generated) / m_total * 100), 100U) << "%]";

		/// Get the family configuration
		uint family_index = dist(m_rng);
		FamilyConfig& new_config = family_config.at(family_index);

		/// Make the configuration into reality
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
	cerr << "\rGenerating households [100%]...\n";
}

void PopulationGenerator::makeCities() {
	m_next_id = 1;
	auto cities_config = m_props.get_child("POPULATION.CITIES");
	uint size_check = 0;

	uint generated = 0;
	uint to_generate = distance(cities_config.begin(), cities_config.end());
	for (auto it = cities_config.begin(); it != cities_config.end(); it++) {
		cerr << "\rGenerating cities [" << min(uint(double(generated) / to_generate * 100), 100U) << "%]";
		if (it->first == "CITY") {
			string name = it->second.get<string>("<xmlattr>.name");
			int size = it->second.get<int>("<xmlattr>.pop");
			double latitude = it->second.get<double>("<xmlattr>.lat");
			double longitude = it->second.get<double>("<xmlattr>.lon");
			size_check += size;

			SimpleCity new_city;
			new_city.m_max_size = size;
			new_city.m_current_size = 0;
			new_city.m_id = m_next_id;
			new_city.m_name = name;
			m_next_id++;
			new_city.m_coord.m_longitude = longitude;
			new_city.m_coord.m_latitude = latitude;

			m_cities.push_back(new_city);
		}
		generated++;
	}
	cerr << "\rGenerating cities [100%]...\n";

	/// Important, make sure the vector is sorted (biggest to smallest)!
	auto compare_city_size = [](const SimpleCity& a, const SimpleCity b) { return a.m_max_size > b.m_max_size; };
	sort (m_cities.begin(), m_cities.end(), compare_city_size);
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
	// Do NOT reset the id counter (cities and villages will be treated as one)
	auto village_config = m_props.get_child("POPULATION.VILLAGES");
	double village_radius_factor = village_config.get<double>("<xmlattr>.radius");
	GeoCoordinate middle = getCityMiddle();
	double radius = getCityRadius(middle);
	uint city_population = getCityPopulation();
	int unassigned_population = m_people.size() - city_population;
	int unassigned_population_progress = unassigned_population;

	/// Get the configuration of the villages (relative occurrence, minimum and maximum population)
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

	/// Depending on the relative occurrence of a village, choose this village
	AliasDistribution village_type_dist {fractions};
	const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
	while (unassigned_population > 0) {
		cerr << "\rGenerating villages [" << min(uint(double(unassigned_population_progress - unassigned_population) / unassigned_population_progress * 100), 100U) << "%]";
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

		/// Make sure this isn't a duplicate coordinate
		auto same_coordinate_village = [&](const SimpleCluster& cl) {return cl.m_coord == new_village.m_coord;};
		auto same_coordinate_city = [&](const SimpleCity& cl) {return cl.m_coord == new_village.m_coord;};

		auto it_villages = find_if(m_villages.begin(), m_villages.end(), same_coordinate_village);
		auto it_cities = find_if(m_cities.begin(), m_cities.end(), same_coordinate_city);

		if (it_villages == m_villages.end() && it_cities == m_cities.end()) {
			m_villages.push_back(new_village);
			unassigned_population -= new_village.m_max_size;
		}
	}
	cerr << "\rGenerating villages [100%]...\n";
}

void PopulationGenerator::placeHouseholds() {
	uint city_pop = getCityPopulation();
	uint village_pop = getVillagePopulation();
	uint total_pop = village_pop + city_pop;	/// Note that this number may slightly differ from other "total pop" numbers

	/// Get the relative occurrences of both the villages and cities => randomly choose an index in this vector based on that
	/// Note that the vector consists of 2 parts: the first one for the cities, the second one for the villages, keep this in mind when generating the random index
	vector<double> fractions;
	for (SimpleCity& city: m_cities) {
		fractions.push_back(double(city.m_max_size) / double(total_pop));
	}

	for (SimpleCluster& village: m_villages) {
		fractions.push_back(double(village.m_max_size) / double(total_pop));
	}

	AliasDistribution village_city_dist {fractions};
	int i = 0;
	for (SimpleHousehold& household: m_households) {
		cerr << "\rPlacing households [" << min(uint(double(i) / m_households.size() * 100), 100U) << "%]";
		uint index = village_city_dist(m_rng);
		if (index < m_cities.size()) {
			/// A city has been chosen
			SimpleCity& city = m_cities.at(index);
			city.m_current_size += household.m_indices.size();
			for (uint& person_index: household.m_indices) {
				m_people.at(person_index).m_coord = city.m_coord;
			}
			m_locations[make_pair(ClusterType::Household, household.m_id)] = city.m_coord;

		} else {
			/// A village has been chosen
			SimpleCluster& village = m_villages.at(index - m_cities.size());
			village.m_current_size += household.m_indices.size();
			for (uint& person_index: household.m_indices) {
				m_people.at(person_index).m_coord = village.m_coord;
			}
			m_locations[make_pair(ClusterType::Household, household.m_id)] = village.m_coord;
		}
		i++;
	}
	cerr << "\rPlacing households [100%]...\n";
}

void PopulationGenerator::makeSchools() {
	/// Note: schools are "assigned" to villages and cities
	m_next_id = 1;
	auto education_config = m_props.get_child("POPULATION.EDUCATION");
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.MANDATORY");
	uint school_size = education_config.get<uint>("MANDATORY.<xmlattr>.total_size");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");

	placeClusters(school_size, min_age, max_age, 1.0, m_mandatory_schools, "schools", ClusterType::School);
}

void PopulationGenerator::makeUniversities() {
	m_next_id = 1;
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE.YOUNG_EMPLOYEE");
	auto university_config = m_props.get_child("POPULATION.EDUCATION.OPTIONAL");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	double fraction = 1.0 - school_work_config.get<double>("<xmlattr>.fraction") / 100.0;
	uint size = university_config.get<uint>("<xmlattr>.total_size");
	uint cluster_size = university_config.get<uint>("<xmlattr>.cluster_size");

	uint intellectual_pop = 0;
	for (uint i = min_age; i <= max_age; i++) {
		intellectual_pop += m_age_distribution[i];
	}

	intellectual_pop = ceil(intellectual_pop * fraction);

	uint needed_universities = ceil(double (intellectual_pop) / size);
	uint placed_universities = 0;
	uint clusters_per_univ = size / cluster_size;	/// Note: not +1 as you cannot exceed a certain amount of students
	uint left_over_cluster_size = size % cluster_size;

	m_optional_schools.clear();

	while (needed_universities > placed_universities) {
		cerr << "\rPlacing Universities [" << min(uint(double(needed_universities) / placed_universities * 100), 100U) << "%]";

		/// Add a university to the list
		/// Note,:a university is a vector of clusters
		vector<SimpleCluster> univ;
		for (uint i = 0; i < clusters_per_univ; i++) {
			SimpleCluster univ_cluster;
			univ_cluster.m_id = m_next_id;
			univ_cluster.m_max_size = cluster_size;
			univ_cluster.m_coord = m_cities.at(placed_universities % m_cities.size()).m_coord;
			m_next_id++;
			univ.push_back(univ_cluster);

			m_locations[make_pair(ClusterType::School, univ_cluster.m_id)] = univ_cluster.m_coord;
		}

		if (left_over_cluster_size > 0) {
			/// Because the last cluster might not fit in the university, this cluster is smaller
			SimpleCluster univ_cluster;
			univ_cluster.m_id = m_next_id;
			univ_cluster.m_max_size = left_over_cluster_size;
			univ_cluster.m_coord = m_cities.at(placed_universities % m_cities.size()).m_coord;
			m_next_id++;
			univ.push_back(univ_cluster);

			m_locations[make_pair(ClusterType::School, univ_cluster.m_id)] = univ_cluster.m_coord;
		}

		m_optional_schools.push_back(univ);
		placed_universities++;
	}
}

void PopulationGenerator::sortWorkplaces() {
	/// Sorts according to the cities (assumes they are sorted in a way that you might desire)
	list<SimpleCluster> result;

	for (SimpleCity& city: m_cities) {
		for (SimpleCluster& workplace: m_workplaces) {
			if (city.m_coord == workplace.m_coord) {
				result.push_back(workplace);
			}
		}
	}

	for (SimpleCluster& village: m_villages) {
		for (SimpleCluster& workplace: m_workplaces) {
			if (village.m_coord == workplace.m_coord) {
				result.push_back(workplace);
			}
		}
	}

	m_workplaces = result;
}

void PopulationGenerator::makeWork() {
	m_next_id = 1;
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE");
	auto work_config = m_props.get_child("POPULATION.WORK");

	uint size = work_config.get<uint>("<xmlattr>.size");
	uint min_age = school_work_config.get<uint>("EMPLOYEE.<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("EMPLOYEE.<xmlattr>.max");
	uint young_min_age = school_work_config.get<uint>("YOUNG_EMPLOYEE.<xmlattr>.min");
	uint young_max_age = school_work_config.get<uint>("YOUNG_EMPLOYEE.<xmlattr>.max");
	double young_fraction = school_work_config.get<double>("YOUNG_EMPLOYEE.<xmlattr>.fraction") / 100.0;
	double fraction = school_work_config.get<double>("<xmlattr>.fraction") / 100.0;

	uint possible_students = 0;
	uint total_old = 0;
	for (SimplePerson& person: m_people) {
		if (person.m_age >= young_min_age && person.m_age <= young_max_age) {
			possible_students++;
		}

		if (person.m_age >= std::max(min_age, young_max_age + 1) && person.m_age <= max_age) {
			total_old++;
		}
	}

	uint total_of_age = possible_students + total_old;
	// Subtract actual students
	uint total_working = total_of_age - ceil(possible_students * (1.0 - young_fraction));
	total_working = ceil(total_working * fraction);

	// Calculate the actual fraction of people between young_min_age and max_age who are working
	double actual_fraction = double(total_working) / total_of_age;

	placeClusters(size, young_min_age, max_age, actual_fraction, m_workplaces, "workplaces", ClusterType::Work);

	/// Make sure the work clusters are sorted from big city to smaller city
	sortWorkplaces();
}

void PopulationGenerator::makeCommunities() {
	/// TODO? Currently not doing the thing with the average communities per person, right now, everyone gets two communities
	m_next_id = 1;
	auto community_config = m_props.get_child("POPULATION.COMMUNITY");
	uint size = community_config.get<uint>("<xmlattr>.size");

	placeClusters(size, 0, 0, 1.0, m_primary_communities, "primary communities", ClusterType::PrimaryCommunity);
	m_next_id = 1;
	placeClusters(size, 0, 0, 1.0, m_secondary_communities, "secondary communities", ClusterType::SecondaryCommunity);
}

void PopulationGenerator::assignToSchools() {
	/// TODO add factor to xml?
	m_next_id = 1;
	auto education_config = m_props.get_child("POPULATION.EDUCATION.MANDATORY");
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.MANDATORY");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	double start_radius = education_config.get<double>("<xmlattr>.radius");
	uint cluster_size = education_config.get<uint>("<xmlattr>.cluster_size");

	double factor = 2.0;

	/// TODO refactor => this is not pretty at all
	for (SimpleCluster& cluster: m_mandatory_schools) {
		SimpleCluster new_cluster;
		new_cluster.m_max_size = cluster_size;
		new_cluster.m_id = m_next_id;
		m_next_id++;
		new_cluster.m_coord = cluster.m_coord;
		m_mandatory_schools_clusters.push_back(vector<SimpleCluster> {new_cluster});
	}

	double current_radius = start_radius;

	uint total = 0;
	uint total_placed = 0;

	for (uint i = min_age; i <= max_age; i++) {
		total += m_age_distribution[i];
	}

	for (SimplePerson& person: m_people) {
		current_radius = start_radius;
		if (person.m_age >= min_age && person.m_age <= max_age) {
			cerr << "\rAssigning children to schools [" << min(uint(double(total_placed) / total * 100), 100U) << "%]";
			total_placed++;

			vector<uint> closest_clusters_indices;

			while (closest_clusters_indices.size() == 0 && m_mandatory_schools.size() != 0) {
				closest_clusters_indices = getClusters(person.m_coord, current_radius, m_mandatory_schools);
				current_radius *= factor;
			}

			AliasDistribution cluster_dist {vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size()))};
			uint index = closest_clusters_indices.at(cluster_dist(m_rng));

			if (m_mandatory_schools_clusters.at(index).back().m_current_size >= m_mandatory_schools_clusters.at(index).back().m_max_size) {
				SimpleCluster new_cluster;
				new_cluster.m_max_size = cluster_size;
				new_cluster.m_id = m_next_id;
				m_next_id++;
				new_cluster.m_coord = m_mandatory_schools_clusters.at(index).back().m_coord;
				m_mandatory_schools_clusters.push_back(vector<SimpleCluster> {new_cluster});
			}

			m_mandatory_schools_clusters.at(index).back().m_current_size++;
			person.m_school_id = m_mandatory_schools_clusters.at(index).back().m_id;
		}
	}
	cerr << "\rAssigning children to schools [100%]...";
}

void PopulationGenerator::assignToUniversities() {
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE.YOUNG_EMPLOYEE");
	auto university_config = m_props.get_child("POPULATION.EDUCATION.OPTIONAL");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	double student_fraction = 1.0 - school_work_config.get<double>("<xmlattr>.fraction") / 100.0;
	double commute_fraction = university_config.get<double>("FAR.<xmlattr>.fraction") / 100.0;
	double radius = university_config.get<double>("<xmlattr>.radius") / 100.0;

	AliasDistribution commute_dist { {commute_fraction, 1.0 - commute_fraction} };
	AliasDistribution student_dist { {student_fraction, 1.0 - student_fraction} };

	uint total = 0;
	uint total_placed = 0;

	for (uint i = min_age; i <= max_age; i++) {
		total += m_age_distribution[i];
	}

	for (SimplePerson& person: m_people) {
		if (person.m_age >= min_age && person.m_age <= max_age && student_dist(m_rng) == 0) {
			cerr << "\rAssigning students to universities [" << min(uint(double(total_placed) / total * 100), 100U) << "%]";
			total_placed++;

			if (commute_dist(m_rng) == 0) {
				/// Commuting student
				assignCommutingStudent(person);
			} else {
				/// Non-commuting student
				assignCloseStudent(person, radius);
			}
		}
	}
	cerr << "\rAssigning students to universities [100%]...\n";
}

void PopulationGenerator::assignCommutingStudent(SimplePerson& person) {
	uint current_city = 0;
	bool added = false;

	while (current_city < m_cities.size() && !added) {
		uint current_univ = current_city;
		while (current_univ < m_optional_schools.size() && !added) {
			for (uint i = 0; i < m_optional_schools.at(current_univ).size(); i++) {
				SimpleCluster& univ_cluster = m_optional_schools.at(current_univ).at(i);
				if (univ_cluster.m_current_size < univ_cluster.m_max_size) {
					univ_cluster.m_current_size++;
					person.m_school_id = univ_cluster.m_id;
					added = true;
					break;
				}
			}
			current_univ += m_cities.size();
		}
		current_city++;
	}

	if (!added) {
		cout << "EXCEPT1\n";
	}
}

void PopulationGenerator::assignCloseStudent(SimplePerson& person, double start_radius) {
	double factor = 2.0;
	double current_radius = start_radius;
	bool added = false;
	vector<uint> closest_clusters_indices;

	while (!added) {

		// Note how getting the distance to the closest univ is the same as getting the distance to the closest city
		// This is because their vectors are in the same order!
		closest_clusters_indices = getClusters(person.m_coord, current_radius, m_cities);

		while (closest_clusters_indices.size() != 0) {
			AliasDistribution dist { vector<double>(closest_clusters_indices.size(), 1.0 / closest_clusters_indices.size())};

			uint index = dist(m_rng);
			while (index < m_optional_schools.size() && !added) {
				for (uint i = 0; i < m_optional_schools.at(index).size(); i++) {
					SimpleCluster& univ_cluster = m_optional_schools.at(index).at(i);
					if (univ_cluster.m_current_size < univ_cluster.m_max_size) {
						univ_cluster.m_current_size++;
						person.m_school_id = univ_cluster.m_id;
						added = true;
						break;
					}
				}
				index += m_cities.size();
			}

			if (added) {
				break;
			} else {
				closest_clusters_indices.erase(closest_clusters_indices.begin() + (index % closest_clusters_indices.size()),
					closest_clusters_indices.begin() + (index % closest_clusters_indices.size()) + 1);
			}
		}

		current_radius *= factor;
		if (closest_clusters_indices.size() == m_cities.size() && !added) {
			cout << "EXCEPT2\n";
		}
	}
}

void PopulationGenerator::assignToWork() {
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE");
	auto work_config = m_props.get_child("POPULATION.WORK");
	uint min_age = school_work_config.get<uint>("YOUNG_EMPLOYEE.<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("EMPLOYEE.<xmlattr>.max");
	double unemployment_rate = 1.0 - school_work_config.get<double>("<xmlattr>.fraction") / 100.0;
	double commute_fraction = work_config.get<double>("FAR.<xmlattr>.fraction") / 100.0;
	double radius = work_config.get<double>("FAR.<xmlattr>.radius") / 100.0;

	AliasDistribution unemployment_dist { {unemployment_rate, 1.0 - unemployment_rate} };
	AliasDistribution commute_dist { {commute_fraction, 1.0 - commute_fraction} };

	uint total = 0;
	uint total_placed = 0;

	for (uint i = min_age; i <= max_age; i++) {
		total += m_age_distribution[i];
	}

	for (SimplePerson& person: m_people) {
		if (m_workplaces.size() == 0) {
			// Stop adding people to workplaces, they are full
			break;
		}

		if (person.m_age >= min_age && person.m_age <= max_age) {
			cerr << "\rAssigning people to workplaces [" << min(uint(double(total_placed) / total * 100), 100U) << "%]";
			total_placed++;
			if (unemployment_dist(m_rng) == 1 && person.m_school_id == 0) {
				if (commute_dist(m_rng) == 0) {
					/// Commuting employee
					assignCommutingEmployee(person);
				} else {
					/// Non-commuting employee
					assignCloseEmployee(person, radius);
				}
			}
		}
	}
	cerr << "\rAssigning people to workplaces [100%]...\n";
}

void PopulationGenerator::assignCommutingEmployee(SimplePerson& person) {
	/// TODO ask question: it states that a full workplace has to be ignored
		/// but workplaces can be in cities and villages where commuting is only in cities  => possible problems with over-employing in cities
	/// Behavior on that topic is currently as follows: do the thing that is requested, if all cities are full, it just adds to the first village in the list

	for (auto it = m_workplaces.begin(); it != m_workplaces.end(); it++) {
		SimpleCluster& workplace = *it;

		if (workplace.m_max_size > workplace.m_current_size) {
			workplace.m_current_size++;
			person.m_work_id = workplace.m_id;

			if (workplace.m_current_size >= workplace.m_max_size) {
				auto it2 = it;
				it2++;
				m_workplaces.erase(it, it2);
			}

			break;
		}
	}
}

void PopulationGenerator::assignCloseEmployee(SimplePerson& person, double start_radius) {
	double factor = 2.0;
	double current_radius = start_radius;

	while (true) {
		vector<list<SimpleCluster>::iterator> closest_clusters_iterators;
		const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
		for (auto it = m_workplaces.begin(); it != m_workplaces.end(); it++) {
			if (calc.getDistance(person.m_coord, it->m_coord) <= current_radius) {
				closest_clusters_iterators.push_back(it);
			}
		}


		if (closest_clusters_iterators.size() != 0) {
			AliasDistribution dist { vector<double>(closest_clusters_iterators.size(), 1.0 / double(closest_clusters_iterators.size())) };
			uint rnd = dist(m_rng);
			auto it = closest_clusters_iterators.at(rnd);
			SimpleCluster& workplace = *it;

			person.m_work_id = workplace.m_id;
			workplace.m_current_size++;

			if (workplace.m_current_size >= workplace.m_max_size) {
				auto it2 = it;
				it2++;
				m_workplaces.erase(it, it2);
			}

			break;
		}
		current_radius *= factor;
	}
}

void PopulationGenerator::assignToCommunities() {
	/// NOTE to self: community vectors are destroyed!
	double start_radius = m_props.get<double>("POPULATION.COMMUNITY.<xmlattr>.radius");
	double factor = 2.0;
	vector<uint> closest_clusters_indices;

	uint total = m_households.size();
	uint total_placed = 0;

	for (SimpleHousehold& household: m_households) {
		cerr << "\rAssigning people to primary community [" << min(uint(double(total_placed) / total * 100), 100U) << "%]";
		total_placed++;

		double current_radius = start_radius;

		while (true) {
			/// Get the clusters within a certain radius
			closest_clusters_indices = getClusters(m_people.at(household.m_indices.at(0)).m_coord, current_radius, m_primary_communities);

			if (closest_clusters_indices.size() == 0) {
				/// Search was unsuccessful, try again
				current_radius *= factor;
			} else {
				/// Search was successfull, uniformly choose a community
				AliasDistribution dist { vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size())) };
				uint index = closest_clusters_indices.at(dist(m_rng));
				SimpleCluster& community = m_primary_communities.at(index);
				for (uint& person_index: household.m_indices) {
					SimplePerson& person = m_people.at(person_index);
					person.m_primary_community = community.m_id;
					community.m_current_size++;
				}

				/// Remove the community if it is full
				if (community.m_current_size >= community.m_max_size) {
					m_primary_communities.erase(m_primary_communities.begin() + index);
				}
				break;
			}
		}
	}
	cerr << "\rAssigning people to primary community [100%]...\n";

	total_placed = 0;

	for (SimpleHousehold& household: m_households) {
		cerr << "\rAssigning people to secondary community [" << min(uint(double(total_placed) / total * 100), 100U) << "%]";
		total_placed++;

		double current_radius = start_radius;

		while (true) {
			/// Get the clusters within a certain radius
			closest_clusters_indices = getClusters(m_people.at(household.m_indices.at(0)).m_coord, current_radius, m_secondary_communities);

			if (closest_clusters_indices.size() == 0) {
				/// Search was unsuccessful, try again
				current_radius *= factor;
			} else {
				/// Search was successfull, uniformly choose a community
				AliasDistribution dist { vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size())) };
				uint index = closest_clusters_indices.at(dist(m_rng));
				SimpleCluster& community = m_secondary_communities.at(index);
				for (uint& person_index: household.m_indices) {
					SimplePerson& person = m_people.at(person_index);
					person.m_secondary_community = community.m_id;
					community.m_current_size++;
				}

				/// Remove the community if it is full
				if (community.m_current_size >= community.m_max_size) {
					m_secondary_communities.erase(m_secondary_communities.begin() + index);
				}
				break;
			}
		}
	}
	cerr << "\rAssigning people to secondary community [100%]...\n";
}