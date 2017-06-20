
#pragma once
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <utility>
#include <fstream>

#include "sim/Simulator.h"
#include "core/Cluster.h"
#include "util/Observer.h"

using boost::property_tree::ptree;
using std::string;
using std::pair;
using std::ofstream;


namespace stride {

class ClusterSaver : public util::Observer<Simulator> {
public:
	ClusterSaver(string file_name);

	virtual void update(const Simulator& sim) {
		saveClustersCSV(sim);
		// saveClustersJSON(sim);
		m_sim_day++;
	}

private:
	void saveClustersCSV(const Simulator& sim) const;
	inline void saveClusterCSV(const Cluster& cluster, ofstream& csv_file) const;

	void saveClustersJSON(const Simulator& sim) const;
	pair<ptree, ptree> getClusterJSON(const Cluster& cluster) const;

	unsigned int m_sim_day = 0;
	string m_file_name;

};


}
