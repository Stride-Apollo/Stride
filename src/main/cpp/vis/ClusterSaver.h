
#pragma once
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <utility>
#include <fstream>

#include "sim/LocalSimulatorAdapter.h"
#include "core/Cluster.h"
#include "util/Observer.h"

using boost::property_tree::ptree;
using std::string;
using std::pair;
using std::ofstream;
using std::vector;


namespace stride {

class ClusterSaver : public util::Observer<LocalSimulatorAdapter> {
public:
	ClusterSaver(string file_name);

	virtual void update(const LocalSimulatorAdapter& sim) {
		saveClustersCSV(sim);
		m_sim_day++;
	}

private:
	/// Saves cluster information for Households (aggregated), Primary Communities and Secondary Communities.
	void saveClustersCSV(const LocalSimulatorAdapter& local_sim) const;

	/// Saves a single cluster.
	inline void saveClusterCSV(const Cluster& cluster, ofstream& csv_file) const;

	/// Aggregates the vector of given clusters according to their GeoLocation, and saves them.
	void saveAggrClustersCSV(const vector<Cluster>& households, ofstream& csv_file) const;

	/// Saves an aggregated cluster. The clusters that need to be aggregated are given by the indices.
	void saveClusterGroup(const vector<Cluster>& households, const vector<unsigned int> indices, ofstream& csv_file) const;

	// Deprecated
	void saveClustersJSON(const LocalSimulatorAdapter& local_sim) const;
	// Deprecated
	pair<ptree, ptree> getClusterJSON(const Cluster& cluster) const;

private:
	unsigned int m_sim_day = 0;
	string m_file_name;
	string m_file_dir;

};


}
