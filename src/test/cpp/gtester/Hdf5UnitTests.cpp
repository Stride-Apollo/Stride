#include "checkpointing/Saver.h"
#include "sim/SimulatorBuilder.h"
#include "sim/Simulator.h"
#include "pop/Population.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "util/InstallDirs.h"
#include "util/async.h"
#include "util/etc.h"

#include "Hdf5Base.h"

#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>

#include <string>
#include <iostream>

using namespace std;
using namespace stride;
using namespace ::testing;
using namespace H5;

namespace Tests {

class UnitTests__HDF5 : public Hdf5Base {};

/**
 *	Test case that checks the amount of timestaps created in the H5 file,
 *		using checkpointing frequency = 1.
 */
 
TEST_P(UnitTests__HDF5, AmtCheckpoints1) {
	unsigned int num_threads = GetParam();

	unsigned int num_days = 10;
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 1, false));
	std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	local_sim->registerObserver(classInstance, fnCaller);

	local_sim->notify(*local_sim);

	for (unsigned int i = 0; i < num_days; i++) {
		vector<future<bool>> fut_results;
		fut_results.push_back(local_sim->timeStep());
		future_pool(fut_results);
	}

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	dataset.close();
	h5file.close();

	EXPECT_EQ(num_days + 1, hdf5_timesteps[0]);
}

/**
 *	Test case that checks the amount of timestaps created in the H5 file,
 *		using checkpointing frequency = 2.
 */
TEST_P(UnitTests__HDF5, AmtCheckPoints2) {
	unsigned int num_threads = GetParam();

	unsigned int num_days = 10;
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());

	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 2, false));
	std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	local_sim->registerObserver(classInstance, fnCaller);

	local_sim->notify(*local_sim);

	for (unsigned int i = 0; i < num_days; i++) {
		vector<future<bool>> fut_results;
		fut_results.push_back(local_sim->timeStep());
		future_pool(fut_results);
	}

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	dataset.close();
	h5file.close();

	EXPECT_EQ((num_days/2) + 1, hdf5_timesteps[0]);
}

/**
 *	Test case that checks the amount of timestaps created in the H5 file,
 *		using checkpointing frequency = 0.
 *	With this frequency, the hdf5 saver should save no timesteps automically.
 */
TEST_P(UnitTests__HDF5, AmtCheckPoints3) {
	unsigned int num_threads = GetParam();

	unsigned int num_days = 10;
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();


	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, false);
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());

	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 0, false));
	std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	local_sim->registerObserver(classInstance, fnCaller);
	classInstance->forceSave(*local_sim);

	for (unsigned int i = 0; i < num_days; i++) {
		vector<future<bool>> fut_results;
		fut_results.push_back(local_sim->timeStep());
		future_pool(fut_results);
	}
	classInstance->forceSave(*local_sim, num_days);

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	dataset.close();
	h5file.close();

	EXPECT_EQ(2U, hdf5_timesteps[0]);
}

/**
 *	Test that checks the stored config data.
 */
TEST_F(UnitTests__HDF5, CheckConfigTree) {
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	Saver saver = Saver(h5filename.c_str(), pt_config, 1, false);

	// Retrieve the configuration settings from the Hdf5 file.
	StrType h5_str (0, H5T_VARIABLE);
	CompType typeConfData(sizeof(ConfDataType));
	typeConfData.insertMember(H5std_string("conf_content"), HOFFSET(ConfDataType, conf_content), h5_str);
	typeConfData.insertMember(H5std_string("disease_content"), HOFFSET(ConfDataType, disease_content), h5_str);
	typeConfData.insertMember(H5std_string("age_contact_content"), HOFFSET(ConfDataType, age_contact_content), h5_str);
	typeConfData.insertMember(H5std_string("holidays_content"), HOFFSET(ConfDataType, holidays_content), h5_str);
	ConfDataType configData[1];

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	defer(h5file.close());
	DataSet dataset = h5file.openDataSet("configuration/configuration");
	defer(dataset.close());
	dataset.read(configData, typeConfData);

	istringstream iss(configData[0].conf_content);
	ptree pt_config_hdf5;
	xml_parser::read_xml(iss, pt_config_hdf5);

	/// Check if the stored data conforms to the original data
	#define ASSERT_CONFIG_EQUAL(type, key) ASSERT_EQ(pt_config.get<type>(key), pt_config_hdf5.get<type>(key))
	ASSERT_CONFIG_EQUAL(string, "run.population_file");
	ASSERT_CONFIG_EQUAL(double, "run.r0");
	ASSERT_CONFIG_EQUAL(unsigned int, "run.num_days");
	ASSERT_CONFIG_EQUAL(unsigned int, "run.rng_seed");
	ASSERT_CONFIG_EQUAL(double, "run.seeding_rate");
	ASSERT_CONFIG_EQUAL(double, "run.immunity_rate");
	ASSERT_CONFIG_EQUAL(string, "run.output_prefix");
	ASSERT_CONFIG_EQUAL(unsigned int, "run.num_participants_survey");
	ASSERT_CONFIG_EQUAL(string, "run.start_date");
	ASSERT_CONFIG_EQUAL(string, "run.log_level");
	ASSERT_CONFIG_EQUAL(unsigned int, "run.generate_person_file");
	ASSERT_CONFIG_EQUAL(string, "run.checkpointing_file");
	ASSERT_CONFIG_EQUAL(int, "run.checkpointing_frequency");
	#undef ASSERT_CONFIG_EQUAL
}


/**
 *	Simple test case to test constructor.
 */
TEST_F(UnitTests__HDF5, CreateSaver) {
	auto pt_config = getConfigTree();
	const string h5filename = "testOutput.h5";
	Saver saver = Saver(h5filename.c_str(), pt_config, 1, false);
}


/**
 *	Test that checks the amount of persons stored in the hdf5 file.
 */
TEST_F(UnitTests__HDF5, CheckAmtPersons) {
	const string h5filename = "testOutput.h5";
	// const string h5filename = "/tmp/testOutput.h5";
	auto pt_config = getConfigTree();

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, 1, false);
	auto local_sim = make_shared<LocalSimulatorAdapter>(sim.get());
	auto classInstance = std::make_shared<Saver>
		(Saver(h5filename.c_str(), pt_config, 1, false));
	std::function<void(const LocalSimulatorAdapter&)> fnCaller = std::bind(&Saver::update, classInstance, std::placeholders::_1);
	local_sim->registerObserver(classInstance, fnCaller);
	local_sim->notify(*local_sim);

	vector<future<bool>> fut_results;
	fut_results.push_back(local_sim->timeStep());
	future_pool(fut_results);
	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);

	DataSet* dataset = new DataSet(h5file.openDataSet("personsTI"));
	DataSpace dataspace = dataset->getSpace();
	const int amt_dims = dataspace.getSimpleExtentNdims();

	EXPECT_EQ(amt_dims, 1);

	hsize_t dims[amt_dims];
	dataspace.getSimpleExtentDims(dims, NULL);
	dataspace.close();
	dataset->close();
	delete dataset;
	h5file.close();

	EXPECT_EQ(sim->getPopulation().get()->m_original.size(), dims[0]);
}


#if UNIPAR_IMPL == UNIPAR_DUMMY
	unsigned int threads_hdf5unitTests[] { 1U };
#else
	unsigned int threads_hdf5unitTests[] { 1U, 4U };
#endif

INSTANTIATE_TEST_CASE_P(HDF5UnitTestsAmtCheckpoints, UnitTests__HDF5, ::testing::ValuesIn(threads_hdf5unitTests));

}
