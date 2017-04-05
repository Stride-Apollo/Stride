#include "checkpointing/Saver.h"
#include "sim/SimulatorBuilder.h"
#include "sim/Simulator.h"
#include "pop/Population.h"
#include "checkpointing/customDataTypes/ConfDataType.h"

#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>
#include <omp.h>

#include <string>
#include <iostream>

using namespace std;
using namespace stride;
using namespace ::testing;
using namespace H5;

namespace Tests {

class HDF5UnitTests : public ::testing::TestWithParam<unsigned int> {
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
		return config_tree;
	}
	virtual ~HDF5UnitTests() {}


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
};

const string		HDF5UnitTests::g_population_file				= "pop_nassau.csv";
const double		HDF5UnitTests::g_r0								= 11.0;
const unsigned int	HDF5UnitTests::g_num_days						= 50U;
const unsigned int	HDF5UnitTests::g_rng_seed						= 1U;
const double		HDF5UnitTests::g_seeding_rate					= 0.002;
const double		HDF5UnitTests::g_immunity_rate					= 0.8;
const string		HDF5UnitTests::g_disease_config_file			= "disease_measles.xml";
const string		HDF5UnitTests::g_output_prefix					= "testHdf5";
const string		HDF5UnitTests::g_holidays_file					= "holidays_none.json";
const unsigned int	HDF5UnitTests::g_num_participants_survey		= 10;
const string		HDF5UnitTests::g_start_date						= "2017-01-01";
const string		HDF5UnitTests::g_age_contact_matrix_file		= "contact_matrix_average.xml";
const string		HDF5UnitTests::g_log_level						= "None";
const unsigned int	HDF5UnitTests::g_generate_person_file			= 0;



/**
 *	Test case that checks the amount of timestaps created in the H5 file,
 *		using checkpointing frequency = 1.
 */
TEST_P(HDF5UnitTests, AmtCheckpoints1) {
	unsigned int num_threads = GetParam();
	omp_set_num_threads(num_threads);
	omp_set_schedule(omp_sched_static,1);

	unsigned int num_days = 50;
	string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 1, false));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	for (unsigned int i = 0; i < num_days; i++) {
		sim->timeStep();
	}

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	h5file.close();

	EXPECT_EQ(num_days-1, hdf5_timesteps[0]);
}

/**
 *	Test case that checks the amount of timestaps created in the H5 file,
 *		using checkpointing frequency = 2.
 */
TEST_P(HDF5UnitTests, AmtCheckPoints2) {
	unsigned int num_threads = GetParam();
	omp_set_num_threads(num_threads);
	omp_set_schedule(omp_sched_static,1);

	unsigned int num_days = 50;
	string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 2, false));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	for (unsigned int i = 0; i < num_days; i++) {
		sim->timeStep();
	}

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	h5file.close();

	EXPECT_EQ((num_days/2)-1, hdf5_timesteps[0]);
}

/**
 *	Test case that checks the amount of timestaps created in the H5 file,
 *		using checkpointing frequency = 0.
 *	With this frequency, the hdf5 saver should only store 1 timestep.
 */
TEST_P(HDF5UnitTests, AmtCheckPoints3) {
	unsigned int num_threads = GetParam();
	omp_set_num_threads(num_threads);
	omp_set_schedule(omp_sched_static,1);

	unsigned int num_days = 50;
	string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 0, false));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	for (unsigned int i = 0; i < num_days; i++) {
		sim->timeStep();
	}

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	h5file.close();

	EXPECT_EQ(0U, hdf5_timesteps[0]);
}

/**
 *	Test that checks the stored config data.
 */
TEST_F(HDF5UnitTests, CheckConfigTree) {
	string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	Saver saver = Saver(h5filename.c_str(), pt_config, 1, false);


	// ================================================
	// Constructing datatype to read configuration data
	// ================================================

	/*CompType typeConf = CompType(sizeof(ConfDataType));
	typeConf.insertMember(H5std_string("checkpointing_frequency"),
						  HOFFSET(ConfDataType, checkpointing_frequency), PredType::NATIVE_UINT);
	typeConf.insertMember(H5std_string("rng_seed"), HOFFSET(ConfDataType, rng_seed), PredType::NATIVE_ULONG);
	typeConf.insertMember(H5std_string("r0"), HOFFSET(ConfDataType, r0), PredType::NATIVE_UINT);
	typeConf.insertMember(H5std_string("seeding_rate"),
						  HOFFSET(ConfDataType, seeding_rate), PredType::NATIVE_DOUBLE);
	typeConf.insertMember(H5std_string("immunity_rate"),
						  HOFFSET(ConfDataType, immunity_rate), PredType::NATIVE_DOUBLE);
	typeConf.insertMember(H5std_string("num_days"), HOFFSET(ConfDataType, num_days), PredType::NATIVE_UINT);
	StrType tid1(0, H5T_VARIABLE);
	typeConf.insertMember(H5std_string("output_prefix"), HOFFSET(ConfDataType, output_prefix), tid1);
	typeConf.insertMember(H5std_string("generate_person_file"),
						  HOFFSET(ConfDataType, generate_person_file), PredType::NATIVE_HBOOL);
	typeConf.insertMember(H5std_string("num_participants_survey"),
						  HOFFSET(ConfDataType, num_participants_survey), PredType::NATIVE_UINT);
	typeConf.insertMember(H5std_string("start_date"), HOFFSET(ConfDataType, start_date), tid1);
	typeConf.insertMember(H5std_string("log_level"), HOFFSET(ConfDataType, log_level), tid1);
	*/
	// ================================================


	/// Check if the stored data conforms to the original data
	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("configuration/configuration");

	ConfDataType config[1];
	//dataset.read(config, typeConf);
	/*ASSERT_EQ(g_r0, config->r0);
	ASSERT_EQ(g_rng_seed, config->rng_seed);
	ASSERT_EQ(g_immunity_rate, config->immunity_rate);
	ASSERT_EQ(g_num_days, config->num_days);
	ASSERT_EQ(g_output_prefix, config->output_prefix);
	ASSERT_EQ(g_generate_person_file, config->generate_person_file);
	ASSERT_EQ(g_num_participants_survey, config->num_participants_survey);
	ASSERT_EQ(g_start_date, config->start_date);
	ASSERT_EQ(g_log_level, config->log_level);*/
}


/**
 *	Simple test case to test constructor.
 */
TEST_F(HDF5UnitTests, CreateSaver) {
	auto pt_config = getConfigTree();
	string output = "testOutput.h5";
	Saver saver = Saver(output.c_str(), pt_config, 1, false);
}


TEST_F(HDF5UnitTests, CheckAmtPersons) {
	string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, 1, false);
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 0, false));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	sim->timeStep();

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("personsTI");

	hsize_t dims[1];
	dataset.getSpace().getSimpleExtentDims(dims, NULL);
	h5file.close();

	// TODO uncomment when completed in saver
	// EXPECT_EQ(sim->getPopulation()->size(), dims[0]);
}



#ifdef _OPENMP
	unsigned int threads[] { 1U , 4U, 8U};
#else
	unsigned int threads[] { 1U };
#endif

INSTANTIATE_TEST_CASE_P(AmtCheckpointsThreads, HDF5UnitTests, ::testing::ValuesIn(threads));


}