
#include "Runner.h"

#include <iostream>
#include <exception>
#include <thread>
#include <cstddef>
#include <spdlog/spdlog.h>
#include "util/InstallDirs.h"
#include "sim/LocalSimulatorAdapter.h"
#include "sim/RemoteSimulatorSender.h"
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
        : m_config_file(config_file), m_mode(mode), m_uses_mpi(false), m_timestep(timestep), m_world_rank(0) {
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
	parseConfig();
	m_output_dir = base_dir / m_name;
}

void Runner::parseConfig() {
	pt::read_xml(m_config_file, m_config, pt::xml_parser::trim_whitespace);
	for (auto& override: m_overrides) {
		m_config.put(override.first, override.second);
	}
	bool has_raw_population = false;
	for (auto& it: m_config.get_child("run.regions")) {
		if (it.first == "region") {
			pt::ptree& region_config = it.second;
			string name = region_config.get<string>("<xmlattr>.name");
			m_region_configs[name] = region_config;
			m_region_order.push_back(name);

			if (region_config.count("raw_population") != 0) {
				has_raw_population = true;
			}
		}
	}
	if (m_region_configs.size() == 0) {
		throw runtime_error("You need at least one region");
	} else if (m_region_configs.size() > 1 && has_raw_population) {
		throw runtime_error("One of the regions does not contain the necessary information to work in a multi region environment (districts, cities, ...)");
	}
	m_travel_schedule = m_config.get<string>("run.regions.<xmlattr>.travel_schedule", "");
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
			cout << "    - '" << it.first << "' running locally" << endl;
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
		cout << "--> Created new output directory at " << output_dir << "." << endl;
	} else {
		cout << "--> Using existing output directory at " << output_dir << ", will overwrite." << endl;
	}

	for (auto& it: m_region_configs) {
		boost::optional<string> remote = it.second.get_optional<string>("remote");
		if (remote) {
			#ifdef MPI_USED
				makeSetupStruct();
				initMpi();
			#else
				throw runtime_error("MPI support is not enabled in this build");
			#endif
			break;
		}
	}

	for (auto& it: m_region_configs) {
		cout << "\r--> Initializing simulators [" << i << "/" << m_region_configs.size() << "]";
		i++;
		cout.flush();

		boost::optional<string> remote = it.second.get_optional<string>("remote");
		pt::ptree sim_config = getRegionsConfig({it.first});
		string sim_name = sim_config.get<string>("run.regions.region.<xmlattr>.name");
		if (m_is_master) {
			if (not remote) {
				addLocalSimulator(sim_name, sim_config);
			} else {
				addRemoteSimulator(sim_name, sim_config);
			}
		} else {
 			if (not remote) {
 				// This is a simulator running at the master
 				// TODO: get Master's contact info?
 				// Then, do MPI stuff
 				addRemoteSimulator(sim_name, sim_config);
 			} else {
 				if (sim_name == "TODO get process ID") {
 					// This is our (unique) local simulator
 					addLocalSimulator(sim_name, sim_config);
 				} else {
 					// This is just another remote simulator
 					addRemoteSimulator(sim_name, sim_config);
 				}
 			}
		}
	}
	
	cout << endl;

	if (m_uses_mpi and m_local_simulators.size() > 1) {
		throw runtime_error("You can't have multiple simulators in one system when working with MPI");
	}

	// Also set up the Coordinator
	// TODO allow a single simulator without schedule
	if (m_world_rank == 0) m_coord = make_shared<Coordinator>(m_async_simulators, m_travel_schedule, m_config);
}

shared_ptr<Simulator> Runner::addLocalSimulator(const string& name, const boost::property_tree::ptree& config) {
	auto sim = SimulatorBuilder::build(config);
	sim->m_name = config.get<string>("run.regions.region.<xmlattr>.name");

	if (m_mode == RunMode::Replay || m_mode == RunMode::Extend) {
		// adjust the state of the simulator
		Hdf5Loader loader = Hdf5Loader(hdf5Path(name).string().c_str());

		int timestep = m_mode == RunMode::Extend ?
					   loader.getLastSavedTimestep() : m_timestep;

		loader.loadFromTimestep(timestep, sim);
	}

	initOutputs(*sim.get());
	m_local_simulators[name] = sim;
	m_async_simulators[name] = make_shared<LocalSimulatorAdapter>(sim);
	return sim;
}

void Runner::initMpi() {
#ifdef MPI_USED
	if (not m_uses_mpi) {
		int provided = 0;
		MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
		if (provided != MPI_THREAD_SERIALIZED) throw runtime_error("We need serialized thread support in MPI");
		MPI_Comm_rank(MPI_COMM_WORLD, &m_world_rank);
		MPI_Comm_size(MPI_COMM_WORLD, &m_world_size);
		m_uses_mpi = true;
		m_is_master = (m_world_rank == 0);

		char processor_name[MPI_MAX_PROCESSOR_NAME];
		int name_len;
		MPI_Get_processor_name(processor_name, &name_len);
		m_processor_name = std::string(processor_name);

		vector<SimulatorWorldrank> worldranks;
		for (auto& it: m_region_configs) {
			boost::optional<string> remote = it.second.get_optional<string>("remote");
			if (remote) {
				if (remote.get() == m_processor_name) {
					// TODO Anthony: send the SimulatorWorldrank!
					SimulatorWorldrank setup {it.second.get<string>("<xmlattr>.name"), m_world_rank};
					MPI_Send(&setup, 1, MPI_INT, 0, 20, MPI_COMM_WORLD); // Tag 20 = setup related MPI message
				}
			} else if (m_is_master) {
				worldranks.emplace_back(it.second.get<string>("<xmlattr>.name"), 0);
			}
		}

		if (m_is_master) {
			// TODO Anthony: collect all SimulatorWorldranks into worldranks
			for (int i=1; i<m_world_size; i++){
				SimulatorWorldrank data {"",0};
				MPI_Recv(&data, 1, MPI_INT, i, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				worldranks.push_back(data);
			}
			for (SimulatorWorldrank& swr: worldranks) {
				char* name = swr.simulator_name;
				int rank = swr.world_rank;
				bool unique_name = m_worldranks.left.find(name) == m_worldranks.left.end();
				bool unique_rank = m_worldranks.right.find(rank) == m_worldranks.right.end();
				if (unique_name and unique_rank) {
					m_worldranks.insert(decltype(m_worldranks)::value_type(name, rank));
				}
			}

			if (m_worldranks.size() != m_region_configs.size()) {
				throw runtime_error(to_string(m_worldranks.size()) + " simulators but "
									+ to_string(m_region_configs.size()) + " regions.");
			}

			// TODO Anthony: send m_worldranks to every other node
		} else {
			// TODO Anthony: collect m_worldranks and set it accordingly
		}
	}
#endif
}

void Runner::makeSetupStruct(){
#ifdef MPI_USED
		// TODO fix string, now its a single char
		MPI_Datatype type[2] = {MPI_CHAR, MPI_INT};
		/** Number of occurence of each type */
		int blocklen[2] = {20, 1};
		/** Position offset from struct starting address */
		MPI_Aint disp[2];
		disp[0] = offsetof(SimulatorWorldrank, simulator_name);
		disp[1] = offsetof(SimulatorWorldrank, world_rank);
		/** Create the type */
		MPI_Type_create_struct(2, blocklen, disp, type, &m_setup_message);
		MPI_Type_commit(&m_setup_message);
#endif
}

shared_ptr<AsyncSimulator> Runner::addRemoteSimulator(const string& name, const boost::property_tree::ptree& config) {
	// TODO: DO MPI STUFF
	boost::optional<string> remote = config.get_optional<string>("run.regions.region.remote");
	cout << "Remote:" << remote << endl;
	m_async_simulators[name] = make_shared<RemoteSimulatorSender>(name, stoi(remote.get()));
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

	auto checkpointing = m_config.get_child_optional("run.outputs.checkpointing");
	if (checkpointing) {
		int freq = checkpointing.get().get<int>("<xmlattr>.frequency");
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

	auto visualization = m_config.get_child_optional("run.outputs.visualization");
	if (visualization) {
		// TODO Vis output, how does it work?
		// We need to save to m_output_dir / ...<something that makes sense in the context of visualisation>...
		// See hdf5Path for inspiration, but since we only consider output (whereas hdf5 is also input) there's
		// no need to write a separate method for it.
		std::string vis_output_dir = fs::system_complete(m_output_dir / (string("vis_") + sim.m_name)).string();
		auto vis_saver = make_shared<ClusterSaver>("vis_output", "vis_pop_output", "vis_facility_output", vis_output_dir);
		auto fn = bind(&ClusterSaver::update, vis_saver, std::placeholders::_1);
		sim.registerObserver(vis_saver, fn);
		vis_saver->update(sim);
		m_vis_savers[sim.m_name] = vis_saver;
	}

	// TODO other output files (cases, summary, persons, participants survey?)
}

void Runner::run() {
	if (m_is_master) {
		if (m_uses_mpi)
			cout << m_processor_name << " is running,";
		else
			cout << "We are running locally,";
		cout << " printing infected/adopted." << endl;
		int num_days = m_config.get<int>("run.num_days");
		cout << endl << "day  | ";
		int total = 0;
		for (auto& it: m_region_configs) {
			cout << it.first;
			for (int i=15 - it.first.size(); i>0; i--) cout << ' ';
			cout << " | ";
		}

		cout << endl << "-----+";
		for (auto& it: m_region_configs) {
			cout << "-----------------+";
		}
		cout << endl;

		for (int day = 0; day < m_timestep + num_days; day++) {
			cout << setw(4) << day << " | ";
			vector<SimulatorStatus> results = m_coord->timeStep();
			for (auto& ss: results) {
				cout << setw(7) << ss.infected << " " << setw(7) << ss.adopted << " | ";
			}
			cout << endl;
		}
	} else {
		cout << m_processor_name << " awaits messages." << endl;
	}

	// TODO only save at last timestep if freq == 0
	// for (auto& it: m_hdf5_savers) {
	// 	Simulator& sim = *m_local_simulators[it.first];
	// 	it.second->forceSave(sim, m_timestep + num_days);
	// }

#ifdef MPI_USED
	// Close the MPI environment properly
	if (m_uses_mpi) {
		if (m_world_rank == 0){
			// Send message from system 0 (coordinator) to all other systems to terminate their listening thread
			for (int i = 0; i < m_world_size; i++) MPI_Send(nullptr, 0, MPI_INT, i, 10, MPI_COMM_WORLD);
		}
		m_listen_thread.join(); // Join and terminate listening thread
		MPI_Finalize();
	}
#endif
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
	return fs::system_complete(m_output_dir / (string("cp_") + name + ".h5"));
}

pt::xml_writer_settings<string> Runner::g_xml_settings = pt::xml_writer_make_settings<string>('\t', 1);
