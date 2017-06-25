/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of the Simulator class.
 */

#include "SimulatorBuilder.h"

#include "calendar/Calendar.h"
#include "core/Infector.h"
#include "pop/Population.h"
#include "pop/PopulationBuilder.h"
#include "util/InstallDirs.h"
#include "util/GeoCoordinate.h"
#include "util/StringUtils.h"
#include "util/TransportFacilityReader.h"
#include "core/Cluster.h"
#include "core/ClusterType.h"
#include "behaviour/information_policies/InformationPolicy.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <map>
#include <string>
#include <utility>
#include <algorithm>

namespace stride {

using namespace std;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace stride::util;

shared_ptr<Simulator> SimulatorBuilder::build(const ptree& pt_config) {
	// Disease file.
	ptree pt_disease;
	const string file_name_d = pt_config.get<string>("run.disease.config");
	const auto file_path_d = (InstallDirs::getDataDir() /= file_name_d);
	if (!is_regular_file(file_path_d)) {
		throw runtime_error(std::string(__func__) + "> No file " + file_path_d.string());
	}
	read_xml(file_path_d.string(), pt_disease);

	// Contact file.
	ptree pt_contact;
	const string file_name_c = pt_config.get("run.age_contact_matrix_file", "contact_matrix.xml");
	const auto file_path_c = (InstallDirs::getDataDir() /= file_name_c);
	if (!is_regular_file(file_path_c)) {
		throw runtime_error(string(__func__) + "> No file " + file_path_c.string());
	}
	read_xml(file_path_c.string(), pt_contact);

	// Done.
	return build(pt_config, pt_disease, pt_contact);
}

shared_ptr<Simulator> SimulatorBuilder::build(const ptree& pt_config,
											  const ptree& pt_disease, const ptree& pt_contact) {
	auto sim = make_shared<Simulator>();

	// initialize config ptree.
	sim->m_config_pt = pt_config;

	// initialize track_index_case policy
	sim->m_track_index_case = pt_config.get("run.track_index_case", false);

	// initialize number of threads.
	sim->m_num_threads = pt_config.get("run.num_threads", -1);
	sim->m_parallel.setNumThreads(sim->m_num_threads);

	// initialize calendar.
	sim->m_calendar = make_shared<Calendar>(pt_config);

	// get log level.
	const string l = pt_config.get<string>("run.outputs.log.<xmlattr>.level", "None");
	sim->m_log_level = isLogMode(l) ? toLogMode(l) : throw runtime_error(
			string(__func__) + "> Invalid input for LogMode.");

	// Rng's.
	int seed = pt_config.get<int>("run.regions.region.rng_seed");
	sim->m_rng = make_shared<util::Random>(seed);

	// Build population.
	ptree pt_pop;
	if (pt_config.get("run.regions.region.population", "") == "") {
		pt_pop.put("population.people", pt_config.get<string>("run.regions.region.raw_population"));
	} else {
		read_xml((InstallDirs::getDataDir() / pt_config.get<string>("run.regions.region.population")).string(),
		pt_pop, xml_parser::trim_whitespace);
	}
	sim->m_population = PopulationBuilder::build(pt_config, pt_disease, pt_pop, *sim->m_rng);
	sim->m_config_pop = pt_pop;

	// Get the next id for new travellers
	unsigned int max_id = 0;
	const Population& population = *(sim->m_population.get());
	for (auto& person: population) {
		max_id = std::max(uint(max_id), uint(person.getId()));
	}

	sim->m_next_id = max_id + 1;

	// Get the new household id for travellers
	max_id = 0;
	for (auto& hh: sim->m_households) {
		max_id = std::max(uint(max_id), uint(hh.getId()));
	}

	sim->m_next_hh_id = max_id + 1;

	// Initialize districts.
	initializeDistricts(sim, pt_pop);

	// Initialize the facilities
	initializeFacilities(sim, pt_pop);

	// Initialize clusters.
	initializeClusters(sim, pt_pop);

	// initialize disease profile.
	sim->m_disease_profile.initialize(pt_config, pt_disease);

	// Initialize contact profiles.
	Cluster::addContactProfile(ClusterType::Household, ContactProfile(ClusterType::Household, pt_contact));
	Cluster::addContactProfile(ClusterType::School, ContactProfile(ClusterType::School, pt_contact));
	Cluster::addContactProfile(ClusterType::Work, ContactProfile(ClusterType::Work, pt_contact));
	Cluster::addContactProfile(ClusterType::PrimaryCommunity,
							   ContactProfile(ClusterType::PrimaryCommunity, pt_contact));
	Cluster::addContactProfile(ClusterType::SecondaryCommunity,
							   ContactProfile(ClusterType::SecondaryCommunity, pt_contact));

	// Done.
	return sim;
}

void SimulatorBuilder::initializeClusters(shared_ptr<Simulator> sim, const boost::property_tree::ptree& pt_config) {
	// Determine number of clusters.
	unsigned int max_id_households = 0U;
	unsigned int max_id_school_clusters = 0U;
	unsigned int max_id_work_clusters = 0U;
	unsigned int max_id_primary_community = 0U;
	unsigned int max_id_secondary_community = 0U;
	Population& population = *sim->m_population;

	for (const auto& p : population) {
		max_id_households = std::max(max_id_households, p.getClusterId(ClusterType::Household));
		max_id_school_clusters = std::max(max_id_school_clusters, p.getClusterId(ClusterType::School));
		max_id_work_clusters = std::max(max_id_work_clusters, p.getClusterId(ClusterType::Work));
		max_id_primary_community = std::max(max_id_primary_community, p.getClusterId(ClusterType::PrimaryCommunity));
		max_id_secondary_community = std::max(max_id_secondary_community,
											  p.getClusterId(ClusterType::SecondaryCommunity));
	}

	// Keep separate id counter to provide a unique id for every cluster.
	unsigned int cluster_id = 1;

	string cluster_filename = "";

	// Get the name of the file with the locations of the clusters
	boost::optional<const ptree&> cluster_locations_config = pt_config.get_child_optional("population.clusters");
	if(cluster_locations_config) {
		cluster_filename = pt_config.get<string>("population.clusters");
	}

	map<pair<ClusterType, uint>, GeoCoordinate> locations = initializeLocations(cluster_filename);

	for (size_t i = 0; i <= max_id_households; i++) {
		sim->m_households.emplace_back(Cluster(cluster_id, ClusterType::Household, locations[make_pair(ClusterType::Household, i)]));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_school_clusters; i++) {
		sim->m_school_clusters.emplace_back(Cluster(cluster_id, ClusterType::School, locations[make_pair(ClusterType::School, i)]));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_work_clusters; i++) {
		sim->m_work_clusters.emplace_back(Cluster(cluster_id, ClusterType::Work, locations[make_pair(ClusterType::Work, i)]));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_primary_community; i++) {
		sim->m_primary_community.emplace_back(Cluster(cluster_id, ClusterType::PrimaryCommunity, locations[make_pair(ClusterType::PrimaryCommunity, i)]));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_secondary_community; i++) {
		sim->m_secondary_community.emplace_back(Cluster(cluster_id, ClusterType::SecondaryCommunity, locations[make_pair(ClusterType::SecondaryCommunity, i)]));
		cluster_id++;
	}

	// TODO add cities and villages

	// Cluster id '0' means "not present in any cluster of that type".
	for (auto& p: population) {
		const auto hh_id = p.getClusterId(ClusterType::Household);
		if (hh_id > 0) {
			sim->m_households[hh_id].addPerson(&p);
		}
		const auto sc_id = p.getClusterId(ClusterType::School);
		if (sc_id > 0) {
			sim->m_school_clusters[sc_id].addPerson(&p);
		}
		const auto wo_id = p.getClusterId(ClusterType::Work);
		if (wo_id > 0) {
			sim->m_work_clusters[wo_id].addPerson(&p);
		}
		const auto primCom_id = p.getClusterId(ClusterType::PrimaryCommunity);
		if (primCom_id > 0) {
			sim->m_primary_community[primCom_id].addPerson(&p);
		}
		const auto secCom_id = p.getClusterId(ClusterType::SecondaryCommunity);
		if (secCom_id > 0) {
			sim->m_secondary_community[secCom_id].addPerson(&p);
		}
	}
}

void SimulatorBuilder::initializeDistricts(shared_ptr<Simulator> sim, const boost::property_tree::ptree& pt_config) {
	// Get the name of the file with the locations of the clusters
	boost::optional<const ptree&> districts_config = pt_config.get_child_optional("population.districts");
	if(districts_config) {
		string district_filename = pt_config.get<string>("population.districts");
		double influence_speed = pt_config.get<double>("population.sphere_of_influence.<xmlattr>.speed");
		double influence_minimum = pt_config.get<double>("population.sphere_of_influence.<xmlattr>.min");
		unsigned int influence_size = pt_config.get<unsigned int>("population.sphere_of_influence.<xmlattr>.size");

		// Check for the correctness of the file
		const auto file_path = InstallDirs::getDataDir() /= district_filename;
		if (!is_regular_file(file_path)) {
			throw runtime_error(string(__func__)
								+ ">Districts file " + file_path.string() + " not present. Aborting.");
		}

		// Open the file
		boost::filesystem::ifstream districts_file;
		districts_file.open(file_path.string());
		if (!districts_file.is_open()) {
			throw runtime_error(string(__func__)
								+ "> Error opening districts file " + file_path.string());
		}

		// Parse the file and fill the map
		string line;
		getline(districts_file, line); // step over file header

		while (getline(districts_file, line)) {
			auto values = StringUtils::split(line, ",");

			// Remove the quotes
			values[1].erase(values[1].begin());
			values[1].erase(values[1].end() - 1);

			// Check for duplicates
			auto search_duplicate = [&] (const District& district) {return district.getName() == values[1];};
			if (find_if(sim->m_districts.cbegin(), sim->m_districts.cend(), search_duplicate) == sim->m_districts.cend()) {
				sim->m_districts.push_back(District(values[1],
												influence_size,
												influence_speed,
												influence_minimum,
												GeoCoordinate(StringUtils::fromString<double>(values[6]),
																StringUtils::fromString<double>(values[7]))));
			}
		}
	}
}


map<pair<ClusterType, uint>, GeoCoordinate> SimulatorBuilder::initializeLocations(string filename) {
	map<pair<ClusterType, uint>, GeoCoordinate> cluster_locations;

	if (filename != "") {
		// Check for the correctness of the file
		const auto file_path = InstallDirs::getDataDir() /= filename;
		if (!is_regular_file(file_path)) {
			throw runtime_error(string(__func__)
								+ ">Cluster location file " + file_path.string() + " not present. Aborting.");
		}

		// Open the file
		boost::filesystem::ifstream locations_file;
		locations_file.open(file_path.string());
		if (!locations_file.is_open()) {
			throw runtime_error(string(__func__)
								+ "> Error opening cluster location file " + file_path.string());
		}

		// Parse the file and fill the map
		string line;
		getline(locations_file, line); // step over file header

		while (getline(locations_file, line)) {
			const auto values = StringUtils::split(line, ",");
			// NOTE: if the values are invalid, it will be zero/Null due to StringUtils/ClusterType
			cluster_locations[make_pair(toClusterType(values[1]), StringUtils::fromString<unsigned int>(values[0]))] = GeoCoordinate(
							StringUtils::fromString<double>(values[2]),
							StringUtils::fromString<double>(values[3]));
		}
	}
	return cluster_locations;
}

void SimulatorBuilder::initializeFacilities(shared_ptr<Simulator> sim, const boost::property_tree::ptree& pt_config) {
	// Get the name of the file with the names of the facilities

	if (!pt_config.get_child_optional("population.cities"))
		return;

	for (auto pot_city : pt_config.get_child("population.cities")) {
		if (pot_city.first == "city") {
			ptree city_tree = pot_city.second;
			for (auto it : city_tree) {
				if (it.first == "airport") {
					// Check if the referenced district exists
					auto find_district = [&] (const District& district) {return district.getName() == pot_city.second.get<string>("<xmlattr>.name");};
					auto le_city = find_if(sim->m_districts.begin(), sim->m_districts.end(), find_district);

					// If the district exists, add the facility
					if (le_city != sim->m_districts.end()) {
						le_city->addFacility(it.second.get<string>("<xmlattr>.name"));
					}
				}
			}
		}
	}
}

}
