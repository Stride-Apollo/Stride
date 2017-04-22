
#include "util/InstallDirs.h"
#include "checkpointing/Saver.h"
#include "checkpointing/Loader.h"
#include "pop/Population.h"
#include "sim/SimulatorBuilder.h"
#include "sim/Simulator.h"

#include <boost/property_tree/ptree.hpp>
#include <omp.h>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace stride;
using namespace ::testing;
using namespace H5;

namespace Tests {


class HDF5ScenarioTests : public ::testing::TestWithParam<unsigned int> {
public:
	virtual void setUp() {}
	virtual void TearDown() {}
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



/// This test is similair to the parametrized test below, but alot more efficient (only runs original simulation once).
/// Still keeping the test below for debugging reasons 
TEST_F(HDF5ScenarioTests, StartFromCheckpoints) {
	unsigned int num_threads = 1;
	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	omp_set_num_threads(num_threads);
	omp_set_schedule(omp_sched_static,1);

	const string h5filename = (util::InstallDirs::getCurrentDir() /= "/tests/testOutput.h5").string();
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 1, false));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	vector<unsigned int> cases_original;
	for (unsigned int i = 0; i < NUM_DAYS; i++) {
		sim->timeStep();
		cases_original.push_back(sim->getPopulation()->getInfectedCount());
	}


	int num_cases_original = cases_original.at(NUM_DAYS-1);

	for (unsigned int i = 1; i < NUM_DAYS; i++) {
		Loader loader(h5filename.c_str(), num_threads);
		auto sim_checkpointed = SimulatorBuilder::build(loader.getConfig(), loader.getDisease(), loader.getContact(), num_threads, false);
		loader.loadFromTimestep(i, sim_checkpointed);

		ASSERT_EQ(cases_original.at(i), sim_checkpointed->getPopulation()->getInfectedCount());
		// cout << "Infected count after loading from last timestep: " << sim_checkpointed->getPopulation()->getInfectedCount() << endl;

		for (unsigned int j = 0; j < NUM_DAYS - i; j++) {
			sim_checkpointed->timeStep();
		}
		const unsigned int num_cases_checkpointed = sim_checkpointed->getPopulation()->getInfectedCount();

		cout << "Original: " << num_cases_original << ", checkpointed: " << num_cases_checkpointed << endl;
		ASSERT_NEAR(num_cases_original, num_cases_checkpointed, 10000);
		
	}
}

/// TODO remove this test if debugging is over
TEST_P(HDF5ScenarioTests, StartFromCheckpoint) {
	unsigned int num_days_checkpointed = GetParam();

	// TODO maybe parametrize this as well	
	unsigned int num_threads = 1;
	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	omp_set_num_threads(num_threads);
	omp_set_schedule(omp_sched_static,1);

	const string h5filename = (util::InstallDirs::getCurrentDir() /= "/tests/testOutput.h5").string();
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 1, false));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	vector<string> rng_states_step_after_save = vector<string>();
	vector<string> rng_states_step_after_load = vector<string>();

	for (unsigned int i = 0; i < NUM_DAYS; i++) {
		if (i == num_days_checkpointed) {
			sim->unregister(classInstance);
			cout << "Infected count at save: " << sim->getPopulation()->getInfectedCount() << endl;
		}
		// if (i == num_days_checkpointed ) {
		// 	rng_states_step_after_save = sim->getRngStates();
		// }
		sim->timeStep();
	}

	const unsigned int num_cases_original = sim->getPopulation()->getInfectedCount();

	Loader loader(h5filename.c_str(), num_threads);
	auto sim_checkpointed = SimulatorBuilder::build(loader.getConfig(), loader.getDisease(), loader.getContact(), num_threads, false);
	loader.extendSimulation(sim_checkpointed);
	cout << "Infected count after loading from last timestep: " << sim_checkpointed->getPopulation()->getInfectedCount() << endl;

	for (unsigned int i = 0; i < NUM_DAYS - num_days_checkpointed; i++) {
		// if (i == 0) {
		// 	rng_states_step_after_load = sim_checkpointed->getRngStates();
		// }
		sim_checkpointed->timeStep();
	}
	const unsigned int num_cases_checkpointed = sim_checkpointed->getPopulation()->getInfectedCount();

	// cout << "States of rng after one iteration (after saving):" << endl;
	// for (auto state : rng_states_step_after_save) {
	// 	cout << state << endl;
	// }
	// cout << "States of rng after one iteration after loading from save:" << endl;
	// for (auto state : rng_states_step_after_load) {
	// 	cout << state << endl;
	// }
	// for (unsigned int i = 0; i < rng_states_step_after_save.size(); i++) {
	// 	ASSERT_EQ(rng_states_step_after_save.at(i), rng_states_step_after_load.at(i));
	// }

	cout << "Original: " << num_cases_original << ", checkpointed: " << num_cases_checkpointed << endl;
	ASSERT_NEAR(num_cases_original, num_cases_checkpointed, 10000);
}





auto days = [&](unsigned int amt_days){
	vector<unsigned int> days; 
	for (unsigned int i = 1; i < amt_days; i++) days.push_back(i); 
	return days;
};


INSTANTIATE_TEST_CASE_P(StartFromCheckpointX, HDF5ScenarioTests, ::testing::ValuesIn(days(NUM_DAYS)));

}
