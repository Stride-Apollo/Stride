
#pragma once
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <utility>

#include "sim/LocalSimulatorAdapter.h"
#include "core/Cluster.h"
#include "util/Observer.h"

using boost::property_tree::ptree;
using std::string;
using std::pair;


namespace stride {

class ClusterSaver : public util::Observer<LocalSimulatorAdapter> {
public:
	ClusterSaver(string file_name);

	virtual void update(const LocalSimulatorAdapter& sim) {
		saveClustersJSON(sim);
		m_sim_day++;
	}

private:
	void saveClustersJSON(const LocalSimulatorAdapter& local_sim) const;
	pair<ptree, ptree> getClusterJSON(const Cluster& cluster) const;

	unsigned int m_sim_day = 0;
	string m_file_name;

};


}
