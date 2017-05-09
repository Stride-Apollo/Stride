

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <utility>
#include <iostream>

#include "vis/ClusterSaver.h"
#include "util/InstallDirs.h"

using boost::property_tree::ptree;
using boost::property_tree::write_json;
using std::string;


namespace stride {

ClusterSaver::ClusterSaver(string file_name) : m_sim_day(0), m_file_name(file_name) {}

void ClusterSaver::saveClustersJSON(const LocalSimulatorAdapter& local_sim) const {
	ptree clusters;
	clusters.put("type", "FeatureCollection");
	{
		ptree clusters_primaries;
		for (unsigned int i = 0; i < local_sim.m_sim->m_primary_community.size(); i++) {
			pair<ptree, ptree> cluster_pair = this->getClusterJSON(local_sim.m_sim->m_primary_community.at(i), i);
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
	write_json(util::InstallDirs::getOutputDir().string() + "/" + m_file_name + std::to_string(m_sim_day) + ".json", clusters);
}

pair<ptree, ptree> ClusterSaver::getClusterJSON(const Cluster& cluster, const unsigned int id) const {
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

	size_t size = cluster.getSize();
	size_t infected_count = cluster.getInfectedCount();
	double ratio = (size == 0 ? 0 : (double) infected_count / size);
	if (infected_count == 0) {
		ratio = -1;
	}

	cluster_properties.put("id", id);
	cluster_properties.put("size", size);
	cluster_properties.put("infected", infected_count);
	cluster_properties.put("infected_percent", ratio);

	return std::make_pair(cluster_geometry, cluster_properties);
}

}
