#ifdef HDF5_USED
#include "checkpointing/Hdf5Saver.h"
#include "sim/SimulatorBuilder.h"
#include "sim/Simulator.h"
#include "pop/Population.h"
#include "checkpointing/datatypes/ConfigDataType.h"
#include "util/InstallDirs.h"
#include "util/async.h"
#include "util/etc.h"

#include "Hdf5Base.h"

#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>

#include <cmath>
#include <iostream>
#include <string>

using namespace std;
using namespace stride;
using namespace ::testing;
using namespace H5;

namespace Tests {

class UnitTests__HDF5 : public Hdf5Base {};
/**
 *	Test case that checks the amount of timestaps created in the H5 file.
 */

TEST_P(UnitTests__HDF5, AmtCheckpoints) {
	unsigned int checkpointing_frequency = GetParam();

	unsigned int num_days = 10;
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config);
	auto saverInstance = std::make_shared<Hdf5Saver>
		(Hdf5Saver(h5filename.c_str(), pt_config, checkpointing_frequency));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Hdf5Saver::update, saverInstance, std::placeholders::_1);
	sim->registerObserver(saverInstance, fnCaller);

	saverInstance->forceSave(*sim);

	for (unsigned int i = 0; i < num_days; i++) {
		sim->timeStep();
	}

	if (checkpointing_frequency == 0) {
		saverInstance->forceSave(*sim, num_days);
	}

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("amt_timesteps");
	unsigned int hdf5_timesteps[1];
	dataset.read(hdf5_timesteps, PredType::NATIVE_UINT);
	dataset.close();
	h5file.close();


	unsigned int amt_expected = 0;
	// Special case for frequency = 0
	if (checkpointing_frequency == 0) {
		amt_expected = 2;
	} else {
		amt_expected = floor(num_days / checkpointing_frequency) + 1;
	}
	EXPECT_EQ(amt_expected, hdf5_timesteps[0]);
}



/**
 *	Test that checks the stored config data.
 */
TEST_F(UnitTests__HDF5, CheckConfigTree) {
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	Hdf5Saver hdf5_saver = Hdf5Saver(h5filename.c_str(), pt_config, 1);

	/// Retrieve the configuration settings from the Hdf5 file.
	StrType h5_str (0, H5T_VARIABLE);
	ConfigDataType configData[1];

	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);
	DataSet dataset = h5file.openDataSet("Configuration/configuration");
	dataset.read(configData, ConfigDataType::getCompType());
	istringstream iss(configData[0].m_config_content);
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
	Hdf5Saver hdf5_saver = Hdf5Saver(h5filename.c_str(), pt_config, 1);
}


/**
 *	Test that checks the amount of persons stored in the hdf5 file.
 */
TEST_F(UnitTests__HDF5, CheckAmtPersons) {
	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config);
	auto classInstance = std::make_shared<Hdf5Saver>
		(Hdf5Saver(h5filename.c_str(), pt_config, 1));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Hdf5Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);
	sim->notify(*sim);

	sim->timeStep();
	H5File h5file (h5filename.c_str(), H5F_ACC_RDONLY);

	DataSet dataset = DataSet(h5file.openDataSet("person_time_independent"));
	DataSpace dataspace = dataset.getSpace();
	const int amt_dims = dataspace.getSimpleExtentNdims();

	EXPECT_EQ(amt_dims, 1);

	hsize_t dims[amt_dims];
	dataspace.getSimpleExtentDims(dims, NULL);
	dataspace.close();
	dataset.close();
	h5file.close();

	EXPECT_EQ(sim->getPopulation().get()->m_original.size(), dims[0]);
}


unsigned int checkpointing_frequencies[] { 1U, 2U, 0U };

INSTANTIATE_TEST_CASE_P(HDF5UnitTestsAmtCheckpoints, UnitTests__HDF5, ::testing::ValuesIn(checkpointing_frequencies));
}
#endif
