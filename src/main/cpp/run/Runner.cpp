
#include "Runner.h"

#include <iostream>
#include <exception>
#include "sim/LocalSimulatorAdapter.h"

/*#ifdef HDF5_USED
	#include "sim/SimulatorSetup.h"
#else
	#include "sim/SimulatorBuilder.h"
#endif*/
#include "sim/SimulatorSetup.h"
#include "sim/SimulatorBuilder.h"

#include "util/StringUtils.h"

using namespace stride;
using namespace util;
using namespace run;
using namespace std;

namespace pt = boost::property_tree;

Runner::Runner(const vector<string>& overrides_list, const string& config_file,
			   const RunMode& mode, int timestep)
        : m_config_file(config_file), m_mode(mode), m_timestep(timestep) {
    for (const string& kv: overrides_list) {
        vector<string> parts = StringUtils::split(kv, "=");
        if (parts.size() != 2) {
            throw runtime_error(string("Couldn't parse the override ") + kv);
        }

        string key = string("run.") + StringUtils::trim(parts[0]);
        key = StringUtils::replace(key, "@", "<xmlattr>.");
        m_overrides[key] = StringUtils::trim(parts[1]);
    }
	parseConfig();
}

void Runner::parseConfig() {
	pt::read_xml(m_config_file, m_config, pt::xml_parser::trim_whitespace);
	for (auto& override: m_overrides) {
		m_config.put(override.first, override.second);
	}

	for (auto& it: m_config.get_child("run.regions")) {
		if (it.first == "region") {
			pt::ptree& region_config = it.second;
			string name = region_config.get<string>("<xmlattr>.name");
			m_region_configs[name] = region_config;
			m_region_order.push_back(name);
		}
	}
	if (m_region_configs.size() == 0) {
		throw runtime_error("You need at least one region");
	}
	m_config.get_child("run").erase("regions");

	m_name = m_config.get<string>("run.<xmlattr>.name");
}

void Runner::printInfo() {
	cout << "Configuration info:" << endl;
	cout << "  - name: " << m_name << endl;
	// TODO: I don't know what kind of info would be really useful to print
	cout << "  - regions:" << endl;
	for (auto& it: m_region_configs) {
		boost::optional<string> remote = it.second.get_optional<string>("remote");
		if (remote)
			cout << "    - '" << it.first << "' running at " << remote << endl;
		else
			cout << "    - '" << it.first << " running locally" << endl;
	}
	cout << endl;
}

void Runner::initSimulators() {
	int i = 1;
	for (auto& it: m_region_configs) {
		cout << "\rInitializing simulators [" << i << "/" << m_region_configs.size() << "]";
		cout.flush();

		boost::optional<string> remote = it.second.get_optional<string>("remote");
		pt::ptree sim_config = getRegionsConfig({it.first});
		if (not remote) {
			// build a Simulator...
			#ifdef HDF5_USED
			auto sim = SimulatorSetup(sim_config, string("hdf5_") + m_name,
									  m_mode, m_timestep).getSimulator();
			#else
		  	auto sim = SimulatorBuilder::build(sim_config);
			#endif
			m_local_simulators[it.first] = sim;
			m_async_simulators[it.first] = make_shared<LocalSimulatorAdapter>(sim);
		} else {
			// TODO: DO MPI STUFF
		}
	}
}

void Runner::run() {

}

pt::ptree Runner::getConfig() {
	return getRegionsConfig(m_region_order);
}

pt::ptree Runner::getRegionsConfig(const std::vector<string>& names) {
	pt::ptree regions;
	for (const string& name: names) {
		regions.add_child("region", m_region_configs[name]); // includes the name
	}
	pt::ptree new_config = m_config;
	new_config.add_child("run.regions", regions);
	return new_config;
}

void Runner::write(std::ostream& out, const pt::ptree& tree) {
	pt::write_xml(out, tree, g_xml_settings);
}

pt::xml_writer_settings<string> Runner::g_xml_settings = pt::xml_writer_make_settings<string>('\t', 1);
