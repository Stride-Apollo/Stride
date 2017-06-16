
#include "ProcessConfig.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;

namespace stride {

ProcessConfig::ProcessConfig(const string& filename) {
	if (exists(system_complete(filename))) {
		const auto file_path_config = canonical(system_complete(filename));

		if (!is_regular_file(file_path_config)) {
			throw runtime_error("Configuration file '" +
				system_complete(filename).string() +
				"' is not a regular file. Aborting");
		}

		read_xml(file_path_config.string(), m_base_ptree);
	} else {
		throw runtime_error("Configuration file '" +
				system_complete(filename).string() +
				"' Does not exist. Aborting");
	}
}

void ProcessConfig::extractForest() {
	ptree coordination_ptree =  m_base_ptree.get_child("coordination");
	unsigned int sim_amount = 0;
	const string& simulator_tag = "run";

	for (auto it = m_base_ptree.begin(); it != m_base_ptree.end(); ++it) {
		if (it->first == simulator_tag) {
			++sim_amount;
		}
	}

	for (unsigned int i = 0; i < sim_amount; ++i) {
		ptree simulator_ptree = extractSubTree(m_base_ptree, simulator_tag, i);

		m_config_forest.push_back(mergeTrees(coordination_ptree, simulator_ptree));
	}
}

ptree ProcessConfig::extractSubTree(const ptree& tree, const string& tag, unsigned int occurrence) {
	unsigned int processed = 0;

	for (auto it = tree.begin(); it != tree.end(); ++it) {
		if (it->first == tag) {
			if (processed == occurrence) {
				return it->second;
			}

			++processed;
		}
	}

	throw runtime_error(string(__func__) + "> Requested occurrence exceeds amount of occurrences.");
}

ptree mergeTrees(const ptree& tree1, const ptree& tree2) {
	ptree result;
	result.put_child("", tree1);
	result.put_child("", tree2);

	return result;
}

}