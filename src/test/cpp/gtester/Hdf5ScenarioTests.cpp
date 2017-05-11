
#include "util/async.h"
#include "util/InstallDirs.h"
#include "checkpointing/Saver.h"
#include "checkpointing/Loader.h"
#include "pop/Population.h"
#include "pop/Person.h"
#include "core/Health.h"
#include "sim/SimulatorBuilder.h"
#include "core/Cluster.h"
#include "sim/Simulator.h"

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <iostream>
#include <gtest/gtest.h>
#include <fstream>
#include <vector>


using namespace std;
using namespace stride;
using namespace ::testing;
using namespace H5;

namespace Tests {


class HDF5ScenarioTests : public ::testing::TestWithParam<unsigned int> {
public:
	const boost::property_tree::ptree getConfigTree() const {
		boost::property_tree::ptree config_tree;
		config_tree.put("run.rng_seed", g_rng_seed);
		config_tree.put("run.r0", g_r0);
		config_tree.put("run.seeding_rate", g_seeding_rate);
		config_tree.put("run.immunity_rate", g_immunity_rate);
		config_tree.put("run.population_file", g_population_file);
		config_tree.put("run.num_days", g_num_days);
		config_tree.put("run.output_prefix", g_output_prefix);
		config_tree.put("run.disease_config_file", g_disease_config_file);
		config_tree.put("run.num_participants_survey", g_num_participants_survey);
		config_tree.put("run.start_date", g_start_date);
		config_tree.put("run.holidays_file", g_holidays_file);
		config_tree.put("run.age_contact_matrix_file", g_age_contact_matrix_file);
		config_tree.put("run.log_level", g_log_level);
		config_tree.put("run.generate_person_file", g_generate_person_file);
		config_tree.put("run.checkpointing_file", g_checkpointing_file);
		config_tree.put("run.checkpointing_frequency", g_checkpointing_frequency);
		return config_tree;
	}
	virtual ~HDF5ScenarioTests() {}


protected:
	static const string				g_population_file;
	static const double				g_r0;
	static const unsigned int		g_num_days;
	static const unsigned int		g_rng_seed;
	static const double				g_seeding_rate;
	static const double				g_immunity_rate;
	static const string				g_disease_config_file;
	static const string				g_output_prefix;
	static const string				g_holidays_file;
	static const unsigned int		g_num_participants_survey;
	static const string				g_start_date;
	static const string				g_age_contact_matrix_file;
	static const string				g_log_level;
	static const unsigned int		g_generate_person_file;
	static const string				g_checkpointing_file;
	static const int				g_checkpointing_frequency;
};

const string		HDF5ScenarioTests::g_population_file				= "pop_nassau.csv";
const double		HDF5ScenarioTests::g_r0								= 11.0;
const unsigned int	HDF5ScenarioTests::g_num_days						= 50U;
const unsigned int	HDF5ScenarioTests::g_rng_seed						= 1U;
const double		HDF5ScenarioTests::g_seeding_rate					= 0.002;
const double		HDF5ScenarioTests::g_immunity_rate					= 0.8;
const string		HDF5ScenarioTests::g_disease_config_file			= "disease_measles.xml";
const string		HDF5ScenarioTests::g_output_prefix					= "testHdf5";
const string		HDF5ScenarioTests::g_holidays_file					= "holidays_none.json";
const unsigned int	HDF5ScenarioTests::g_num_participants_survey		= 10;
const string		HDF5ScenarioTests::g_start_date						= "2017-01-01";
const string		HDF5ScenarioTests::g_age_contact_matrix_file		= "contact_matrix_average.xml";
const string		HDF5ScenarioTests::g_log_level						= "None";
const unsigned int	HDF5ScenarioTests::g_generate_person_file			= 0;
const string		HDF5ScenarioTests::g_checkpointing_file				= "testOutput.h5";
const int			HDF5ScenarioTests::g_checkpointing_frequency		= 1;

const unsigned int NUM_DAYS = 50;



TEST_P(HDF5ScenarioTests, StartFromCheckpoints) {
	unsigned int num_threads = GetParam();

	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 1, false));
	std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	local_sim->registerObserver(classInstance, fnCaller);

	local_sim->notify(*local_sim);
	vector<unsigned int> cases_original;
	cases_original.push_back(sim->getPopulation()->getInfectedCount());


	for (unsigned int i = 0; i < NUM_DAYS; i++) {
		vector<future<bool>> fut_results;
		fut_results.push_back(local_sim->timeStep());
		future_pool(fut_results);
		cases_original.push_back(sim->getPopulation()->getInfectedCount());
	}

	const unsigned int num_cases_original = cases_original.at(NUM_DAYS);

	for (unsigned int i = 1; i < NUM_DAYS; i++) {
		Loader loader(h5filename.c_str(), num_threads);
		auto sim_checkpointed = SimulatorBuilder::build(loader.getConfig(), loader.getDisease(), loader.getContact(), num_threads, false);
		loader.loadFromTimestep(i, sim_checkpointed);

		ASSERT_EQ(cases_original.at(i), sim_checkpointed->getPopulation()->getInfectedCount());

		for (unsigned int j = 0; j < NUM_DAYS - i; j++) {
			sim_checkpointed->timeStep();
		}
		const unsigned int num_cases_checkpointed = sim_checkpointed->getPopulation()->getInfectedCount();
		// cout << "Infected original:\t\t" << num_cases_original << endl;
		// cout << "Infected checkpnt:\t\t" << num_cases_checkpointed << endl << endl;

		if (num_threads == 1) {
			ASSERT_EQ(num_cases_original, num_cases_checkpointed);
		} else {
			ASSERT_NEAR(num_cases_original, num_cases_checkpointed, 10000);
		}
	}
}




// auto days = [](unsigned int amt_days){
// 	vector<unsigned int> days;
// 	for (unsigned int i = 1; i < amt_days; i++) days.push_back(i);
// 	return days;
// };

#ifdef _OPENMP
	unsigned int threads_hdf5scenarios[] { 1U, 4U }; // 8 threads maybe unnecessary?
#else
	unsigned int threads_hdf5scenarios[] { 1U };
#endif

INSTANTIATE_TEST_CASE_P(HDF5ScenarioTestsSFC, HDF5ScenarioTests, ::testing::ValuesIn(threads_hdf5scenarios));
//SFC stands for Start From Checkpoints

}