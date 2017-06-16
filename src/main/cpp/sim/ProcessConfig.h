
/**
 * @file
 * Processing of the configuration xml
 */


#pragma once

#include <boost/property_tree/xml_parser.hpp>
#include <string>

using namespace boost::property_tree;
using namespace std;

namespace stride {

class ProcessConfig {
public:
	ProcessConfig(const string& filename);

	const vector<ptree>& getConfigForest() const {
		return m_config_forest;
	}

	const ptree& getConfigTree(unsigned int index) const {
		return m_config_forest.at(index);
	}

private:
	void extractForest();

	ptree extractSubTree(const ptree& tree, const string& tag, unsigned int occurrence);

	ptree mergeTrees(const ptree& tree1, const ptree& tree2);

private:
	vector<ptree> 			m_config_forest;
	ptree 					m_base_ptree;
};

}
