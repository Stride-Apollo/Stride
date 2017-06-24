#pragma once

/**
* @file
* Header file for the Loader class for the checkpointing functionality
*/

#include "H5Cpp.h"
#include "util/Observer.h"
#include "sim/Simulator.h"
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <memory>

using namespace boost::property_tree;
using std::shared_ptr;
using std::string;


namespace stride {

class Hdf5Loader {
public:

public:
	Hdf5Loader(const char* filename);

	/// Load from timestep, if the specified timestep is present in the hdf5 file.
	void loadFromTimestep(unsigned int timestep, shared_ptr<Simulator> sim) const;

	/// Extend the simulation at the last saved timestep.
	void extendSimulation(shared_ptr<Simulator> sim) const {
		loadFromTimestep(this->getLastSavedTimestep(), sim);
	}


public:
	ptree getConfig() const { return m_pt_config; }
	ptree getDisease() const { return m_pt_disease; }
	ptree getContact() const { return m_pt_contact; }

	/// Retrieves the last saved timestep index in the hdf5 file.
	unsigned int getLastSavedTimestep() const;

public:
	/// Extract the configuration files saved in the hdf5 file.
	static void extractConfigs(string filename);


private:
	/// Creates and sets the population for the simulator.
	void setupPopulation(shared_ptr<Simulator> sim) const;

	/// Sets the cluster immune indices to their maximum, so that sortCluster will definitely sort all members.
	void updateClusterImmuneIndices(shared_ptr<Simulator> sim) const;

	/// Reoders the cluster member positions according to the loaded timestep data.
	void loadClusters(H5::H5File& file, string full_dataset_name, std::vector<Cluster>& cluster, shared_ptr<Simulator> sim) const;

	/// Loads the calendar data.
	void loadCalendar(H5::H5File& file, string dataset_name, shared_ptr<Simulator> sim) const;

	/// Load the time dependent person data
	void loadPersonTDData(H5::H5File& file, string dataset_name, shared_ptr<Simulator> sim) const;

	/// Load the rng state (NOTE only happens when stride runs without parallelisation).
	void loadRngState(H5::H5File& file, string dataset_name, shared_ptr<Simulator> sim) const;

	/// Loads the travellers if present.
	void loadTravellers(H5::H5File& file, string dataset_name, shared_ptr<Simulator> sim) const;

	/// Loads the configuration files from the hdf5 file (stored as class attributes).
	void loadConfigs();


private:
	const char* m_filename;

	ptree m_pt_config;
	ptree m_pt_disease;
	ptree m_pt_contact;
};

}