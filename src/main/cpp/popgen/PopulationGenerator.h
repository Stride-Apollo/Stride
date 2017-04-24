#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>
#include <exception>
#include <limits>
#include <list>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "util/AliasDistribution.h"
#include "util/GeoCoordCalculator.h"
#include "popgen/utils.h"
#include <trng/lcg64.hpp>

namespace stride {
namespace popgen {

using namespace std;
using namespace util;

using uint = unsigned int;

/**
 * Generate Populations
 */
class PopulationGenerator {
public:
	/// Constructor: Check if the xml is valid and set up the basic things like a random generator
	PopulationGenerator(const string& filename, bool output = true);

	/// Generates a population, writes the result to the files found in the data directory
	/// Output files are respectively formatted according to the following template files: belgium_population.csv, pop_miami.csv, pop_miami_geo.csv
	void generate(const string& target_cities, const string& target_pop, const string& target_households);

private:
	/// Writes the cities to the file, see PopulationGenerator::generate, recently, the villages have been added to this
	void writeCities(const string& target_cities) const;

	/// Writes the population to the file, see PopulationGenerator::generate
	void writePop(const string& target_pop) const;

	/// Writes the households to the file, see PopulationGenerator::generate
	void writeHouseholds(const string& target_households) const;

	/// Checks the xml on correctness, this includes only semantic errors, no syntax errors
	void chechForValidXML() const;

	/// Sets up the random generator, of course the one specified in the xml
	void makeRNG();

	/// Generates all households (not yet their positions)
	void makeHouseholds();

	/// Generate all cities (without inhabitants)
	void makeCities();

	/// Gets the middle of all cities
	GeoCoordinate getCityMiddle() const;

	/// Returns the distance between the given coordinate and the furthest city (in km)
	double getCityRadius(const GeoCoordinate& coord) const;

	/// Get the population of all the cities combined (if they were on full capacity)
	double getCityPopulation() const;

	/// Get the population of all the villages combined (if they were on full capacity)
	double getVillagePopulation() const;

	/// Generate all villages (without inhabitants)
	void makeVillages();

	/// Assign the households to a city/village
	void placeHouseholds();

	/// Spreads the clusters of people with these constraints over the cities and villages
	/// size: the size of each cluster
	/// min_age and max_age: the category of people that belongs to these clusters (e.g. schools an work have a minimum/maximum age)
	template<typename C>
	void placeClusters(uint size, uint min_age, uint max_age, double fraction, C& clusters, string cluster_name) {
		uint people = 0;

		if (min_age == 0 && max_age == 0) {
			people = m_people.size();
		} else {
			for (uint age = min_age; age <= max_age; age++) {
				people += m_age_distribution[age];
			}
		}

		people = ceil(fraction * people);

		uint needed_clusters = ceil(double(people) / size);
		uint city_village_size = getCityPopulation() + getVillagePopulation();

		/// Get the relative occurrences of both the villages and cities => randomly choose an index in this vector based on that
		/// Note that the vector consists of 2 parts: the first one for the cities, the second one for the villages, keep this in mind when generating the random index
		vector<double> fractions;
		for (const SimpleCity& city: m_cities) {
			fractions.push_back(double(city.m_max_size) / double(city_village_size));
		}

		for (const SimpleCluster& village: m_villages) {
			fractions.push_back(double(village.m_max_size) / double(city_village_size));
		}

		AliasDistribution dist {fractions};
		for (uint i = 0; i < needed_clusters; i++) {
			cerr << "\rPlacing " << cluster_name << " [" << min(uint(double(i) / m_households.size() * 100), 100U) << "%]";
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
		cerr << "\rPlacing " << cluster_name << " [100%]...\n";
	}

	/// Make the schools, place them in a village/city
	void makeSchools();

	/// Make the universities, place them in a city
	void makeUniversities();

	/// Make sure workplaces are sorted: workplaces in bigger cities are in front of workplaces in smaller cities
	void sortWorkplaces();

	/// Make workplaces
	/// Note: due to the fact that the cluster size of workplaces must be respected, the amount of working people will never be above a fixed certain percentage
	void makeWork();

	/// Make the communities
	void makeCommunities();

	/// Get all clusters within a certain radius of the given point, choose those clusters from the given vector of clusters
	template<typename T>
	vector<uint> getClusters(GeoCoordinate coord, double radius, const vector<T> clusters) const {
		vector<uint> result;
		const GeoCoordCalculator& calc = GeoCoordCalculator::getInstance();
		for (uint i = 0; i < clusters.size(); i++) {
			if (calc.getDistance(coord, clusters.at(i).m_coord) <= radius) {
				result.push_back(i);
			}
		}
		return result;
	}

	/// Put children in mandatory schools
	void assignToSchools();

	/// Put students in universities
	void assignToUniversities();

	/// Put one student in a university according to the rules of commuting students
	void assignCommutingStudent(SimplePerson& person);

	/// Put one student in a university according to the rules of students that study close to their home
	void assignCloseStudent(SimplePerson& person, double start_radius);

	/// Assign people to a workplace
	void assignToWork();

	/// Assign one person to a workplace according to the rule of commuting workers
	void assignCommutingEmployee(SimplePerson& person);

	/// Assign one person to a workplace according to the rule of workers that work close to their home
	void assignCloseEmployee(SimplePerson& person, double start_radius);

	/// Assign entire households
	void assignToCommunities();

	boost::property_tree::ptree m_props;							/// > The content of the xml file
	mutable RNGPicker<trng::lcg64> m_rng;										/// > The random generator
	uint m_total;													/// > The total amount of people to be generated (according to the xml)
	vector<SimplePerson> m_people;									/// > All the people
	vector<SimpleHousehold> m_households;							/// > The households (a household is a vector of indices in the vector above)
	vector<SimpleCity> m_cities;									/// > The cities
	vector<SimpleCluster> m_villages;								/// > The villages
	list<SimpleCluster> m_workplaces;								/// > The workplaces
	vector<SimpleCluster> m_primary_communities;					/// > The primary communities
	vector<SimpleCluster> m_secondary_communities;					/// > The primary communities
	vector<SimpleCluster> m_mandatory_schools;						/// > Mandatory schools (Not divided in clusters!!!)
	vector<vector<SimpleCluster> > m_optional_schools;				/// > The universities: One univ is a vector of clusters, ordering is the same as the cities they belong to (using modulo of course)
	bool m_output;

	/// TODO refactor this, it should be this structure from the beginning (see m_mandatory_schools)
	vector<vector<SimpleCluster> > m_mandatory_schools_clusters;	/// > The clusters of the mandatory schools, this should be refactored

	uint m_next_id;													/// > The next id for the nex cluster/school/... ID's are supposed to be unique

	/// Data for visualisation
	// TODO: population density still missing, not sure what to expect
	map<uint, uint> m_age_distribution;								/// > The age distribution (histogram)
	map<uint, uint> m_household_size;								/// > The household size (histogram)
	map<uint, uint> m_work_size;									/// > The size of workplaces (histogram)
};

}
}
