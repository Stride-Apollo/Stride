
#include "util/async.h"
#include "util/InstallDirs.h"
#include "checkpointing/Hdf5Saver.h"
#include "checkpointing/Hdf5Loader.h"
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

#include "Hdf5Base.h"

using namespace std;
using namespace stride;
using namespace ::testing;
#ifdef HDF5_USED
using namespace H5;
#endif
namespace Tests {


class Scenarios__HDF5 : public Hdf5Base {};
const unsigned int NUM_DAYS = 50;

TEST_P(Scenarios__HDF5, StartFromCheckpoints) {
	unsigned int num_threads = GetParam();

	const string h5filename = "testOutput.h5";
	auto pt_config = getConfigTree();
	pt_config.put("run.num_threads", num_threads);

	shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config);
	auto classInstance = std::make_shared<Hdf5Saver>
		(Hdf5Saver(h5filename.c_str(), pt_config, 1));
	std::function<void(const Simulator&)> fnCaller = std::bind(&Hdf5Saver::update, classInstance, std::placeholders::_1);
	sim->registerObserver(classInstance, fnCaller);

	sim->notify(*sim);
	vector<unsigned int> cases_original;
	cases_original.push_back(sim->getPopulation()->getInfectedCount());

	for (unsigned int i = 0; i < NUM_DAYS; i++) {
		sim->timeStep();
		cases_original.push_back(sim->getPopulation()->getInfectedCount());
	}

	const unsigned int num_cases_original = cases_original.at(NUM_DAYS);

	for (unsigned int i = 1; i < NUM_DAYS; i++) {
		Hdf5Loader hdf5_loader(h5filename.c_str());
		auto sim_checkpointed = SimulatorBuilder::build(hdf5_loader.getConfig(), hdf5_loader.getDisease(), hdf5_loader.getContact());
		hdf5_loader.loadFromTimestep(i, sim_checkpointed);

		ASSERT_EQ(cases_original.at(i), sim_checkpointed->getPopulation()->getInfectedCount());

		for (unsigned int j = 0; j < NUM_DAYS - i; j++) {
			sim_checkpointed->timeStep();
		}
		const unsigned int num_cases_checkpointed = sim_checkpointed->getPopulation()->getInfectedCount();

		#if UNIPAR_IMPL == UNIPAR_DUMMY
			ASSERT_EQ(num_cases_original, num_cases_checkpointed);
		#else
			ASSERT_NEAR(num_cases_original, num_cases_checkpointed, 10000);
		#endif
	}
}


#if UNIPAR_IMPL == UNIPAR_DUMMY
	unsigned int threads_hdf5scenarios[] { 1U };
#else
	unsigned int threads_hdf5scenarios[] { 1U, 4U };
#endif

INSTANTIATE_TEST_CASE_P(StartFromCheckpoints, Scenarios__HDF5, ::testing::ValuesIn(threads_hdf5scenarios));
}
