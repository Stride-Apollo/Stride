

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

using boost::property_tree::ptree;
using boost::property_tree::write_json;
using std::map;
using std::ofstream;
using std::setw;
using std::setfill;
using std::string;
using std::stringstream;
using std::vector;


namespace stride {

ClusterSaver::ClusterSaver(string file_name) : m_sim_day(0), m_file_name(file_name) {
	#if defined(__linux__)
		m_file_dir = "vis/resources/app/data/clusterData";
	#elif defined(__APPLE__)
		m_file_dir = "vis/visualization.app/Contents/Resources/app/data/clusterData";
	#endif
	// Sorry windows

	boost::filesystem::path file_path(m_file_dir);
	if (!boost::filesystem::exists(file_path)) {
		throw runtime_error(string("\n\033[0;31mError: \033[0m") +
			"The folder used to store the cluster data is not present.\n" +
			"Make sure you have installed the visualization app by invoking the " +
			"\033[0;35m'make install_vis'\033[0m" + " command.\n");
	}
}



void ClusterSaver::saveClustersCSV(const LocalSimulatorAdapter& local_sim) const {
	ofstream csv_file;
	stringstream ss;
	ss << setfill('0') << setw(5) << m_sim_day;
	string file_name = m_file_dir + "/" + m_file_name + "_" + ss.str() + ".csv";
	csv_file.open(file_name.c_str());

	// Format of the csv file
	csv_file << "id,size,infected,infected_percent,lat,lon,type" << endl;

	for (const auto& cluster : local_sim.m_sim->m_primary_community) {
		this->saveClusterCSV(cluster, csv_file);
	}
	for (const auto& cluster : local_sim.m_sim->m_secondary_community) {
		this->saveClusterCSV(cluster, csv_file);
	}

	this->saveAggrClustersCSV(local_sim.m_sim->m_households, csv_file);

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



void ClusterSaver::saveClustersJSON(const LocalSimulatorAdapter& local_sim) const {
	ptree clusters;
	clusters.put("type", "FeatureCollection");
	{
		ptree clusters_primaries;
		// First cluster is always empty
		for (unsigned int i = 1; i < local_sim.m_sim->m_primary_community.size(); i++) {
			pair<ptree, ptree> cluster_pair = this->getClusterJSON(local_sim.m_sim->m_primary_community.at(i));
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
	// 	for (unsigned int i = 0; i < local_sim.m_sim->m_secondary_community.size(); i++) {
	// 		pair<ptree, ptree> cluster_pair = this->getClusterJSON(local_sim.m_sim->m_secondary_community.at(i), i);
	// 		ptree cluster_secondary;
	// 		cluster_secondary.put("type", "Feature");
	// 		cluster_secondary.push_back(std::make_pair("geometry", cluster_pair.first));
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

}
