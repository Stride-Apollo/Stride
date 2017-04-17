
#include "util/InstallDirs.h"
#include "checkpointing/Saver.h"
#include "checkpointing/Loader.h"
#include "pop/Population.h"
#include "sim/SimulatorBuilder.h"
#include "sim/Simulator.h"

#include <boost/property_tree/ptree.hpp>
#include <omp.h>
#include <string>
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

	for (unsigned int i = 0; i < NUM_DAYS; i++) {
		if (i == num_days_checkpointed) {
			sim->unregister(classInstance);
		}
		sim->timeStep();

	}

	const unsigned int num_cases_original = sim->getPopulation()->getInfectedCount();

	Loader loader(h5filename.c_str(), num_threads);
	auto sim_checkpointed = SimulatorBuilder::build(loader.get_config(), loader.get_disease(), loader.get_contact(), num_threads, false);
	loader.extend_simulation(sim_checkpointed);
	cout << "Infected count after loading from last timestep: " << sim_checkpointed->getPopulation()->getInfectedCount() << endl;

	for (unsigned int i = 0; i < NUM_DAYS - num_days_checkpointed; i++) {
		sim_checkpointed->timeStep();
	}
	const unsigned int num_cases_checkpointed = sim_checkpointed->getPopulation()->getInfectedCount();


	// ASSERT_NEAR(num_cases_original, num_cases_checkpointed, 10000);
}






unsigned int days[NUM_DAYS] {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
							 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
							 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
							 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
							 41, 42, 43, 44, 45, 46, 47, 48, 49};


INSTANTIATE_TEST_CASE_P(StartFromCheckpointX, HDF5ScenarioTests, ::testing::ValuesIn(days));


}