
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "sim/SimulatorRunMode.h"
#include "sim/Simulator.h"

namespace pt = boost::property_tree;

namespace stride {
namespace run {

/// Helper for parsing the config, and starting the simulators
class Runner {
public:
	// The different steps we do:
	Runner(const std::vector<std::string>& overrides_list, const std::string& config_file,
           const RunMode& mode);
	void printInfo();
	void initSimulators();
    void run();

	void writeConfig(std::ostream& out);
	void writeRegionsConfig(std::ostream& out, const std::vector<string>& names);

	static pt::xml_writer_settings<string> g_xml_settings;

private:
	void parseConfig();  // done by constructor

	std::map<std::string, std::string> m_overrides;
	std::string m_config_file;
	RunMode m_mode;
	pt::ptree m_config;
	std::map<std::string, pt::ptree> m_region_configs;
	std::vector<std::string> m_region_order;

	std::map<std::string, shared_ptr<Simulator>> m_local_simulators;

	// Some important configuration keys, used a lot
	std::string m_name;
};

}
}
