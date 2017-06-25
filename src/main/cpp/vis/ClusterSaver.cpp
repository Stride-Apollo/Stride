

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>

#include "vis/ClusterSaver.h"
#include "util/InstallDirs.h"
#include "util/GeoCoordCalculator.h"
#include "core/ClusterType.h"

using boost::property_tree::ptree;
using boost::property_tree::write_json;
using std::map;
using std::ofstream;
using std::setw;
using std::setfill;
using std::string;
using std::stringstream;
using std::vector;
using std::to_string;


namespace stride {

ClusterSaver::ClusterSaver(string file_name, string pop_file_name, string facility_file_name, string output_dir) :
		m_sim_day(0), m_file_name(file_name), m_pop_file_name(pop_file_name), m_facility_file_name(facility_file_name)  {

	/*#if defined(__linux__)
		m_file_dir = "vis/resources/app/data";
	#elif defined(__APPLE__)
		m_file_dir = "vis/visualization.app/Contents/Resources/app/data";
	#endif
	// Sorry windows*/

	m_file_dir = output_dir;

	boost::filesystem::create_directory(boost::filesystem::path(m_file_dir));

	m_pop_file_dir = m_file_dir + "/populationData";
	m_facility_file_dir = m_file_dir + "/transportFacilityData";
	m_file_dir = m_file_dir + "/clusterData";
	// Create the subdirectory if it does not exist.
	if (!boost::filesystem::exists(boost::filesystem::path(m_file_dir))) {
		boost::filesystem::create_directory(boost::filesystem::path(m_file_dir));
	}

	// Create the subdirectory if it does not exist.
	if (!boost::filesystem::exists(boost::filesystem::path(m_pop_file_dir))) {
		boost::filesystem::create_directory(boost::filesystem::path(m_pop_file_dir));
	}

	// Create the subdirectory if it does not exist.
	if (!boost::filesystem::exists(boost::filesystem::path(m_facility_file_dir))) {
		boost::filesystem::create_directory(boost::filesystem::path(m_facility_file_dir));
	}
}



void ClusterSaver::saveClustersCSV(const Simulator& sim) const {
	ofstream csv_file;
	stringstream ss;
	ss << setfill('0') << setw(5) << m_sim_day;
	string file_name = m_file_dir + "/" + m_file_name + "_" + ss.str() + ".csv";
	csv_file.open(file_name.c_str());

	// Format of the csv file
	csv_file << "id,size,infected,infected_percent,lat,lon,type" << endl;

	for (const auto& cluster : sim.m_primary_community) {
		this->saveClusterCSV(cluster, csv_file);
	}
	for (const auto& cluster : sim.m_secondary_community) {
		this->saveClusterCSV(cluster, csv_file);
	}

	this->saveAggrClustersCSV(sim.m_households, csv_file);

	csv_file.close();
}

inline void ClusterSaver::saveClusterCSV(const Cluster& cluster, ofstream& csv_file) const {
	size_t size = cluster.getSize();
	if (size == 0) {
		return;
	}
	size_t infected_count = cluster.getInfectedCount();
	double ratio = (infected_count == 0 ? -1 : (double) infected_count / size);
	GeoCoordinate coords = cluster.getLocation();

	csv_file << cluster.getId() << ',' <<
		size << ',' <<
		infected_count << ',' <<
		ratio << ',' <<
		coords.m_latitude << ',' <<
		coords.m_longitude << ',' <<
		toString(cluster.getClusterType()) << "\n";
}

void ClusterSaver::saveAggrClustersCSV(const vector<Cluster>& households, ofstream& csv_file) const {
	map<GeoCoordinate, vector<unsigned int>> aggregation_mapping;

	for (unsigned int i = 1; i < households.size(); i++) {
		aggregation_mapping[households[i].getLocation()].push_back(i);
	}

	for (auto entry : aggregation_mapping) {
		this->saveClusterGroup(households, entry.second, csv_file);
	}
}

void ClusterSaver::saveClusterGroup(const vector<Cluster>& households, const vector<unsigned int> indices, ofstream& csv_file) const {
	// Use the first id as cluster id
	unsigned int id = households[indices[0]].getId();
	GeoCoordinate coords = households[indices[0]].getLocation();
	string cluster_type = toString(households[indices[0]].getClusterType());

	unsigned int total_size = 0;
	unsigned int total_infected = 0;
	for (auto index : indices) {
		total_size += households[index].getSize();
		total_infected += households[index].getInfectedCount();
	}
	double ratio = (total_infected == 0 ? -1 : (double) total_infected / total_size);

	csv_file << id << ',' <<
		total_size << ',' <<
		total_infected << ',' <<
		ratio << ',' <<
		coords.m_latitude << ',' <<
		coords.m_longitude << ',' <<
		cluster_type << "\n";
}



void ClusterSaver::saveClustersJSON(const Simulator& sim) const {
	ptree clusters;
	clusters.put("type", "FeatureCollection");
	{
		ptree clusters_primaries;
		// First cluster is always empty
		for (unsigned int i = 1; i < sim.m_primary_community.size(); i++) {
			pair<ptree, ptree> cluster_pair = this->getClusterJSON(sim.m_primary_community.at(i));
			ptree cluster_primary;
			cluster_primary.put("type", "Feature");
			cluster_primary.push_back(std::make_pair("geometry", cluster_pair.first));
			cluster_primary.push_back(std::make_pair("properties", cluster_pair.second));

			clusters_primaries.push_back(std::make_pair("", cluster_primary));
		}
		clusters.add_child("features", clusters_primaries);
	}
	// {
	// 	ptree clusters_secondaries;
	// 	for (unsigned int i = 0; i < sim.m_secondary_community.size(); i++) {
	// 		pair<ptree, ptree> cluster_pair = this->getClusterJSON(sim.m_secondary_community.at(i), i);
	// 		ptree cluster_secondary;
	// 		cluster_secondary.put("type", "Feature");
	// 		cluster_secondary.push_back(std::make_pair("geometry", cluster_pair.first));m_pop_file_name
	// 		cluster_secondary.push_back(std::make_pair("properties", cluster_pair.second));
	//
	// 		clusters_secondaries.push_back(std::make_pair("", cluster_secondary));
	// 	}
	// 	clusters.add_child("Secondary_communities", clusters_secondaries);
	// }
	stringstream ss;
	ss << setfill('0') << setw(5) << m_sim_day;
	write_json(util::InstallDirs::getOutputDir().string() + "/" + m_file_name + "_" + ss.str() + ".json", clusters);
}

pair<ptree, ptree> ClusterSaver::getClusterJSON(const Cluster& cluster) const {

	ptree cluster_geometry;
	ptree cluster_properties;

	cluster_geometry.put("type", "Point");
	ptree coordinates;

	GeoCoordinate coords = cluster.getLocation();
	ptree lat;
	ptree lon;
	lat.put("", coords.m_latitude);
	lon.put("", coords.m_longitude);
	coordinates.push_back(std::make_pair("", lon));
	coordinates.push_back(std::make_pair("", lat));
	cluster_geometry.add_child("coordinates", coordinates);

	size_t id = cluster.getId();
	size_t size = cluster.getSize();
	size_t infected_count = cluster.getInfectedCount();
	double ratio = (size == 0 ? 0 : (double) infected_count / size);
	if (infected_count == 0) {
		ratio = -1;
	}
	string cluster_type = toString(cluster.getClusterType());

	cluster_properties.put("id", id);
	cluster_properties.put("size", size);
	cluster_properties.put("infected", infected_count);
	cluster_properties.put("infected_percent", ratio);
	cluster_properties.put("type", cluster_type);

	return std::make_pair(cluster_geometry, cluster_properties);
}

#define SET_CLUSTER_SURFACE(cluster_type) \
{ \
	surface = ClusterCalculator<cluster_type>::calculateSurface(local_sim); \
	if (surface == 0.0) \
		densities.put(toString(cluster_type), 0.0); \
	else \
		densities.put(toString(cluster_type), double(pop_count) / surface); \
}

#define SET_CLUSTER_MAP(cluster_type) \
{ \
	ptree specific_cluster_map; \
	auto cluster_map = ClusterCalculator<cluster_type>::getClusterMap(local_sim); \
	for (auto it = cluster_map.begin(); it != cluster_map.end(); ++it) { \
		if (it->first != 0) { \
			specific_cluster_map.put(to_string(it->first), it->second); \
		} \
	} \
	cluster_sizes.add_child(toString(cluster_type), specific_cluster_map); \
}

void ClusterSaver::savePopDataJSON(const Simulator& local_sim) const {
	stringstream ss;
	ss << setfill('0') << setw(5) << m_sim_day;
	string file_name = m_pop_file_dir + "/" + m_pop_file_name + "_" + ss.str() + ".json";

	ptree pop_data;
	{
		uint pop_count = getPopCount(local_sim);

		ptree densities;

		// Set population densities
		double surface;
		SET_CLUSTER_SURFACE(ClusterType::Household)
		SET_CLUSTER_SURFACE(ClusterType::School)
		SET_CLUSTER_SURFACE(ClusterType::Work)
		SET_CLUSTER_SURFACE(ClusterType::PrimaryCommunity)
		SET_CLUSTER_SURFACE(ClusterType::SecondaryCommunity)

		pop_data.add_child("densities", densities);

		ptree ages;
		// Set ages
		auto age_map = getAgeMap(local_sim);
		for (auto it = age_map.begin(); it != age_map.end(); ++it) {
			ages.put(to_string(it->first), it->second);
		}

		pop_data.add_child("age_map", ages);

		ptree cluster_sizes;

		// Set cluster sizes
		SET_CLUSTER_MAP(ClusterType::Household)
		SET_CLUSTER_MAP(ClusterType::School)
		SET_CLUSTER_MAP(ClusterType::Work)
		SET_CLUSTER_MAP(ClusterType::PrimaryCommunity)
		SET_CLUSTER_MAP(ClusterType::SecondaryCommunity)

		pop_data.add_child("cluster_sizes", cluster_sizes);
	}

	write_json(file_name.c_str(), pop_data);
}

double ClusterSaver::getPopCount(const Simulator& local_sim) const {
	// TODO only people that are not on vacation
	uint total = 0;

	for (const auto& cluster: local_sim.m_primary_community) {
		total += cluster.getSize();
	}

	return total;
}

map<uint, uint> ClusterSaver::getAgeMap(const Simulator& local_sim) const {
	map<uint, uint> result;

	for (const auto& cluster: local_sim.m_primary_community) {
		for (const auto& person: cluster.getMembers()) {
			if (result.find(person.first->getAge()) == result.end()) {
				result[person.first->getAge()] = 0;
			} else {
				++result[person.first->getAge()];
			}
		}
	}

	return result;
}

void ClusterSaver::saveTransportationFacilities(const Simulator& local_sim) const {
	ptree result;
	ptree children;

	for (const auto& district: local_sim.m_districts) {
		for (const auto& facility: district.m_transportations_facilities) {
			ptree facility_config;
			facility_config.put("city", district.getName());
			facility_config.put("name", facility.first);
			facility_config.put("location.lat", district.getLocation().m_latitude);
			facility_config.put("location.lon", district.getLocation().m_longitude);
			facility_config.put("influence", facility.second.getInfluence());
			facility_config.put("passengers_today", facility.second.m_deque.front());
			facility_config.put("passengers_x_days", facility.second.getScore());
			facility_config.put("x_days", facility.second.m_deque.size());

			children.push_back(make_pair("", facility_config));
		}
	}

	result.add_child("facilities", children);

	stringstream ss;
	ss << setfill('0') << setw(5) << m_sim_day;
	string file_name = m_facility_file_dir + "/" + m_facility_file_name + "_" + ss.str() + ".json";
	write_json(file_name.c_str(), result);
}

}
