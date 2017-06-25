
#include "Runner.h"

#include <iostream>
#include <exception>
#include <spdlog/spdlog.h>
#include "util/InstallDirs.h"
#include "sim/LocalSimulatorAdapter.h"
#include "sim/SimulatorSetup.h"
#include "sim/SimulatorBuilder.h"
#include "util/StringUtils.h"

using namespace stride;
using namespace util;
using namespace run;
using namespace std;

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

void Runner::setup() {
	spdlog::set_async_mode(1048576);
}

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
	fs::path base_dir = InstallDirs::getOutputDir();
	m_output_dir = base_dir / m_name;
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

	// Create output dir
	auto base_dir = InstallDirs::getOutputDir();
	auto output_dir = base_dir / m_name;
	bool fresh = fs::create_directory(output_dir);
	if (fresh) {
		cout << "Created new output directory at '" << output_dir << "'." << endl;
	} else {
		cout << "Using existing output directory at '" << output_dir << "', will overwrite." << endl;
	}

	for (auto& it: m_region_configs) {
		cout << "\rInitializing simulators [" << i << "/" << m_region_configs.size() << "]";
		cout.flush();

		boost::optional<string> remote = it.second.get_optional<string>("remote");
		pt::ptree sim_config = getRegionsConfig({it.first});
		if (not remote) {
			// build a Simulator...
			#ifdef HDF5_USED
			auto sim = SimulatorSetup(sim_config, hdf5Path(it.first).string(),
									  m_mode, m_timestep).getSimulator();
			#else
		  	auto sim = SimulatorBuilder::build(sim_config);
			#endif
			initOutputs(sim);
			m_local_simulators[it.first] = sim;
			m_async_simulators[it.first] = make_shared<LocalSimulatorAdapter>(sim);
		} else {
			// TODO: DO MPI STUFF
		}
	}

	// Also set up the Coordinator
	m_coord = make_shared<Coordinator>(m_async_simulators, m_config.get_value("run.regions.<xmlattr>.travel_schedule"));
}

void Runner::initOutputs(Simulator& sim) {
	// There is a difference between the outputs per Simulator and the ones for everything
	// So far, we only have output per Simulator

	// Logs (we had to refactor some stuff for this)
	sim.m_logger = spdlog::rotating_logger_mt(sim.m_name + "_logger",
											  (m_output_dir / (sim.m_name + "_log.txt")).string(),
											  std::numeric_limits<size_t>::max(),
											  std::numeric_limits<size_t>::max());

	sim.m_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging
	// Log level already set

	boost::optional<pt::ptree> checkpointing = m_config.get_optional("run.outputs.checkpointing");
	if (checkpointing) {
		int freq = checkpointing.get().get_value<int>("frequency");
		auto saver = make_shared<Hdf5Saver>(hdf5Path(sim.m_name).string().c_str(), sim.m_config_pt,
											freq, m_mode, m_timestep);
		auto fn = std::bind(&Hdf5Saver::update, saver, std::placeholders::_1);
		sim.registerObserver(saver, fn);
		m_hdf5_savers[sim.m_name] = saver;

		// initial save
		if (!(m_mode == RunMode::Extend && m_timestep != 0)) {
			saver->forceSave(sim);
		}
	}

	boost::optional<pt::ptree> visualization = m_config.get_optional("run.outputs.visualization");
	if (visualization) {
		auto vis_saver = make_shared<ClusterSaver>("cluster_output");
		auto fn = bind(&ClusterSaver::update, vis_saver, std::placeholders::_1);
		sim.registerObserver(vis_saver, fn);
		vis_saver->update(sim);
		m_vis_savers[sim.m_name] = vis_saver;
	}

	// TODO other output files (cases, summary, persons)
}

void Runner::run() {
	int num_days = m_config.get_value<int>("run.num_days")
	for (int day=0; day < m_timestep + num_days; day++) {
		std::cout << "Simulating day: " << setw(5) << day;
		m_coord->timeStep();
		std::cout << "\tDone, infected count: TODO" << endl;
	}

	for (auto& it: m_hdf5_savers) {
		Simulator& sim = *m_local_simulators[it.first];
		it.second->forceSave(sim, m_timestep + num_days);
	}
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

fs::path Runner::hdf5Path(const string& name) {
	return fs::system_complete(m_output_dir / (string("cp_") + name + ".hdf5"));
}

pt::xml_writer_settings<string> Runner::g_xml_settings = pt::xml_writer_make_settings<string>('\t', 1);
