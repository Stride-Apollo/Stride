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
	cout << "after work\n";
	makeCommunities();
	cout << "after comm\n";
	assignToSchools();
	cout << "after school assignment\n";
	assignToUniversities();
	cout << "after uni assignment\n";
	assignToWork();
	cout << "after work assignment\n";
	assignToCommunities();
	cout << "after comm assignment\n";
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

	uint needed_clusters = double(people) / size + 0.5;
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

	placeClusters(school_size, min_age, max_age, 1.0, m_mandatory_schools);
}

void PopulationGenerator::makeUniversities() {
	/// TODO check for overlap between mandatory and optional education
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

	intellectual_pop = intellectual_pop * fraction + 0.5;

	uint needed_universities = double (intellectual_pop) / size + 0.5;
	uint placed_universities = 0;
	uint clusters_per_univ = size / cluster_size;	/// Note: not +1 as you cannot exceed a certain amount of students
	uint left_over_cluster_size = size % cluster_size;

	m_optional_schools.clear();

	while (needed_universities > placed_universities) {
		vector<SimpleCluster> univ;
		for (uint i = 0; i < clusters_per_univ; i++) {
			SimpleCluster univ_cluster;
			univ_cluster.m_id = m_next_id;
			univ_cluster.m_max_size = cluster_size;
			univ_cluster.m_coord = m_cities.at(placed_universities % m_cities.size()).m_coord;
			m_next_id++;
			univ.push_back(univ_cluster);
		}

		if (left_over_cluster_size > 0) {
			SimpleCluster univ_cluster;
			univ_cluster.m_id = m_next_id;
			univ_cluster.m_max_size = left_over_cluster_size;
			univ_cluster.m_coord = m_cities.at(placed_universities % m_cities.size()).m_coord;
			m_next_id++;
			univ.push_back(univ_cluster);
		}

		m_optional_schools.push_back(univ);
		placed_universities++;
	}
}

void PopulationGenerator::sortWorkplaces() {
	vector<SimpleCluster> result;

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
	/// TODO check consistency with working students and stuff
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.EMPLOYABLE");
	auto work_config = m_props.get_child("POPULATION.WORK");

	uint size = work_config.get<uint>("<xmlattr>.size");
	uint min_age = school_work_config.get<uint>("EMPLOYEE.<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("EMPLOYEE.<xmlattr>.max");
	double fraction = school_work_config.get<double>("<xmlattr>.fraction") / 100.0;

	/// TODO subtract people in universities
	placeClusters(size, min_age, max_age, fraction, m_workplaces);
	/// Make sure the work clusters are sorted from big city to smaller city
	sortWorkplaces();
}

void PopulationGenerator::makeCommunities() {
	/// TODO? Currently not doing the thing with the average communities per person, right now, everyone gets two communities
	auto community_config = m_props.get_child("POPULATION.COMMUNITY");
	uint size = community_config.get<uint>("<xmlattr>.size");

	placeClusters(size, 0, 0, 1.0, m_primary_communities);
	placeClusters(size, 0, 0, 1.0, m_secondary_communities);
}

void PopulationGenerator::assignToSchools() {
	/// TODO add factor to xml?
	auto education_config = m_props.get_child("POPULATION.EDUCATION.MANDATORY");
	auto school_work_config = m_props.get_child("POPULATION.SCHOOL_WORK_PROFILE.MANDATORY");
	uint min_age = school_work_config.get<uint>("<xmlattr>.min");
	uint max_age = school_work_config.get<uint>("<xmlattr>.max");
	double start_radius = education_config.get<double>("<xmlattr>.radius");
	uint cluster_size = education_config.get<uint>("<xmlattr>.cluster_size");

	double factor = 2.0;

	/// TODO refactor
	for (SimpleCluster& cluster: m_mandatory_schools) {
		SimpleCluster new_cluster;
		new_cluster.m_max_size = cluster_size;
		new_cluster.m_id = m_next_id;
		m_next_id++;
		new_cluster.m_coord = cluster.m_coord;
		m_mandatory_schools_clusters.push_back(vector<SimpleCluster> {new_cluster});
	}

	double current_radius = start_radius;

	for (SimplePerson& person: m_people) {
		current_radius = start_radius;
		if (person.m_age >= min_age && person.m_age <= max_age) {
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

	for (SimplePerson& person: m_people) {
		if (person.m_age >= min_age && person.m_age <= max_age && student_dist(m_rng) == 0) {
			if (commute_dist(m_rng) == 0) {
				/// Commuting student
				assignCommutingStudent(person);
			} else {
				assignCloseStudent(person, radius);
			}
		}
	}
}

void PopulationGenerator::assignCommutingStudent(SimplePerson& person) {
	uint current_city = 0;
	bool added = false;

	while (current_city < m_cities.size() && !added) {
		uint current_univ = 0;
		while (current_univ < m_optional_schools.size()) {
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
		closest_clusters_indices = getClusters(person.m_coord, current_radius, m_cities);

		for (uint& index: closest_clusters_indices) {

			uint current_univ = index;
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

			if (added) {
				break;
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

	for (SimplePerson& person: m_people) {
		if (person.m_age >= min_age && person.m_age <= max_age && unemployment_dist(m_rng) == 1 && person.m_school_id == 0) {
			if (commute_dist(m_rng) == 0) {
				/// Commuting student
				assignCommutingEmployee(person);
			} else {
				assignCloseEmployee(person, radius);
			}
		}
	}
}

void PopulationGenerator::assignCommutingEmployee(SimplePerson& person) {
	/// TODO ask question: it states that a full workplace has to be ignored
		/// but workplaces can be in cities and villages where commuting is only in cities  => possible problems with over-employing in cities
	/// Behavior on that topic is currently as follows: do the thing that is requested, if all cities are full, it just adds to the first village in the list

	for (SimpleCluster& workplace: m_workplaces) {
		if (workplace.m_max_size > workplace.m_current_size) {
			workplace.m_current_size++;
			person.m_work_id = workplace.m_id;
			break;
		}
	}
}

void PopulationGenerator::assignCloseEmployee(SimplePerson& person, double start_radius) {
	double factor = 2.0;
	double current_radius = start_radius;
	vector<uint> closest_clusters_indices;
	vector<uint> closest_clusters_indices_all;

	while (true) {
		closest_clusters_indices = getClusters(person.m_coord, current_radius, m_workplaces);
		closest_clusters_indices_all = closest_clusters_indices;

		auto full_workplace = [&] (uint& workplace_index){return m_workplaces.at(workplace_index).m_max_size <=  m_workplaces.at(workplace_index).m_current_size;};

		closest_clusters_indices.erase(remove_if(closest_clusters_indices.begin(), closest_clusters_indices.end(), full_workplace), closest_clusters_indices.end());

		current_radius *= factor;

		if (closest_clusters_indices.size() != 0) {
			AliasDistribution dist { vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size())) };
			uint rnd = dist(m_rng);
			uint t = closest_clusters_indices.at(rnd);
			SimpleCluster& workplace = m_workplaces.at(t);

			person.m_work_id = workplace.m_id;
			workplace.m_current_size++;
			break;
		}

		if (closest_clusters_indices_all.size() == m_workplaces.size()) {
			/// TDOD exception
			cout << "EXCEPT3\n";
			exit(0);
		}
	}
}

void PopulationGenerator::assignToCommunities() {
	/// NOTE to self: community vectors are destroyed!
	double start_radius = m_props.get<double>("POPULATION.COMMUNITY.<xmlattr>.radius");
	double factor = 2.0;
	vector<uint> closest_clusters_indices;

	for (SimpleHousehold& household: m_households) {
		double current_radius = start_radius;

		while (true) {
			closest_clusters_indices = getClusters(m_people.at(household.m_indices.at(0)).m_coord, current_radius, m_primary_communities);

			if (closest_clusters_indices.size() == 0) {
				current_radius *= factor;
			} else {
				AliasDistribution dist { vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size())) };
				uint index = closest_clusters_indices.at(dist(m_rng));
				SimpleCluster& community = m_primary_communities.at(index);
				for (uint& person_index: household.m_indices) {
					SimplePerson& person = m_people.at(person_index);
					person.m_primary_community = community.m_id;
					community.m_current_size++;
				}
				if (community.m_current_size >= community.m_max_size) {
					m_primary_communities.erase(m_primary_communities.begin() + index);
				}
				break;
			}
		}
	}

	for (SimpleHousehold& household: m_households) {
		double current_radius = start_radius;

		while (true) {
			closest_clusters_indices = getClusters(m_people.at(household.m_indices.at(0)).m_coord, current_radius, m_secondary_communities);

			if (closest_clusters_indices.size() == 0) {
				current_radius *= factor;
			} else {
				AliasDistribution dist { vector<double>(closest_clusters_indices.size(), 1.0 / double(closest_clusters_indices.size())) };
				uint index = closest_clusters_indices.at(dist(m_rng));
				SimpleCluster& community = m_secondary_communities.at(index);
				for (uint& person_index: household.m_indices) {
					SimplePerson& person = m_people.at(person_index);
					person.m_primary_community = community.m_id;
					community.m_current_size++;
				}
				if (community.m_current_size >= community.m_max_size) {
					m_secondary_communities.erase(m_secondary_communities.begin() + index);
				}
				break;
			}
		}
	}
}