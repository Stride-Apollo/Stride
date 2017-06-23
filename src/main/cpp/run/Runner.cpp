
#include <iostream>
#include <exception>
#include "Runner.h"
#include "util/StringUtils.h"

using namespace stride;
using namespace util;
using namespace run;
using namespace std;

Runner::Runner(const vector<string>& overrides_list, const string& config_file, const RunMode& mode)
        : m_config_file(config_file), m_mode(mode) {
    for (const string& kv: overrides_list) {
        vector<string> parts = StringUtils::split(kv, "=");
        if (parts.size() != 2) {
            throw runtime_error(string("Couldn't parse the override ") + kv);
        }

        string key = string("run.") + StringUtils::trim(parts[0]);
        key = StringUtils::replace(key, "$", "<xmlattr>.");
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
		if (not remote) {
			// build a Simulator...
			//auto sim = make_shared<Simulator>()
			//m_local_simulators[it.first] =
		}
	}
}

void Runner::run() {

}

void Runner::writeConfig(std::ostream& out) {
	writeRegionsConfig(out, m_region_order);
}

void Runner::writeRegionsConfig(std::ostream& out, const std::vector<string>& names) {
	pt::ptree regions;
	for (const string& name: names) {
		regions.add_child("region", m_region_configs[name]); // includes the name
	}
	pt::ptree new_config = m_config;
	new_config.add_child("run.regions", regions);
	pt::write_xml(out, new_config, g_xml_settings);
}

pt::xml_writer_settings<string> Runner::g_xml_settings = pt::xml_writer_make_settings<string>('\t', 1);
