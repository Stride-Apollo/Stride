
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem/path.hpp>
#include "sim/Coordinator.h"
#include "checkpointing/Hdf5Saver.h"
#include "vis/ClusterSaver.h"
#include "sim/SimulatorRunMode.h"
#include "sim/Simulator.h"
#include "sim/AsyncSimulator.h"

namespace stride {
namespace run {

/// Helper for parsing the config, and starting the simulators
class Runner {
public:
	// The different steps we do:
	static void setup();
	Runner(const std::vector<std::string>& overrides_list, const std::string& config_file,
           const RunMode& mode, int timestep);
	void printInfo();
	void initSimulators();
    void run();

	boost::property_tree::ptree getConfig();
	boost::property_tree::ptree getRegionsConfig(const std::vector<string>& names);

	void write(std::ostream& out, const boost::property_tree::ptree&);

	static boost::property_tree::xml_writer_settings<string> g_xml_settings;

private:
	void parseConfig();  // done by constructor
	void initOutputs(Simulator& sim);

	boost::filesystem::path hdf5Path(const string& name);

	std::map<std::string, std::string> m_overrides;
	std::string m_config_file;
	RunMode m_mode;
	int m_timestep;
	boost::property_tree::ptree m_config;
	std::map<std::string, boost::property_tree::ptree> m_region_configs;
	std::vector<std::string> m_region_order;

	std::map<std::string, shared_ptr<Simulator>> m_local_simulators;
	//std::map<std::string, shared_ptr<RemoteSimulatorSender>> m_remote_senders;
	std::map<std::string, shared_ptr<AsyncSimulator>> m_async_simulators;
	std::shared_ptr<Coordinator> m_coord;

	// Some important configuration keys, used a lot
	std::string m_name;
	boost::filesystem::path m_output_dir;
	std::string m_travel_schedule;

	std::map<std::string, std::shared_ptr<Hdf5Saver>> m_hdf5_savers;
	std::map<std::string, std::shared_ptr<ClusterSaver>> m_vis_savers;
};

/// Manages HDF5 etc
/*
class SimulatorRunner {
public:
	SimulatorRunner(const shared_ptr<Simulator>& sim, const shared_ptr<Hdf5Saver>& saver):
		m_sim(sim), m_saver(saver) {}

	shared_ptr<Simulator> m_sim;
	shared_ptr<Hdf5Saver> m_saver;
};
 */

}
}
