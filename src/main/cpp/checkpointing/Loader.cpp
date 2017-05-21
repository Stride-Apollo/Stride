/**
 * @file
 * Source file for the Loader class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iomanip>
#include "Loader.h"
#include "calendar/Calendar.h"
#include "util/InstallDirs.h"
#include "pop/Population.h"
#include "checkpointing/customDataTypes/CalendarDataType.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
#include "sim/SimulatorBuilder.h"
#include "pop/PopulationBuilder.h"
#include "core/Cluster.h"

#include <algorithm>
#include <vector>
#include <string>

using namespace H5;
using namespace boost::property_tree;
using namespace std;
using namespace boost::filesystem;
using namespace stride::util;

namespace stride {


Loader::Loader(const char *filename, unsigned int num_threads) :
 	m_filename(filename), m_num_threads(num_threads) {

	try {
		this->loadConfigs();
		this->loadTrackIndexCase();
	} catch(FileIException error) {
		error.printError();
	}
}

void Loader::loadConfigs() {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);

	DataSet dataset = DataSet(file.openDataSet("configuration/configuration"));
	ConfDataType configData[1];
	dataset.read(configData, ConfDataType::getCompType());
	dataset.close();
	file.close();

	auto getPropTree = [](string xml_content, ptree& dest_pt) {
		istringstream iss;
		iss.str(xml_content);
		xml_parser::read_xml(iss, dest_pt);
	};

	getPropTree(configData[0].conf_content, m_pt_config);
	getPropTree(configData[0].disease_content, m_pt_disease);
	getPropTree(configData[0].age_contact_content, m_pt_contact);
}

void Loader::loadTrackIndexCase() {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	DataSet dataset = DataSet(file.openDataSet("track_index_case"));
	int track[1];
	dataset.read(track, PredType::NATIVE_INT);
	dataset.close();
	file.close();

	m_track_index_case = track[0];
}




void Loader::loadFromTimestep(unsigned int timestep, std::shared_ptr<Simulator> sim) const {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);

	stringstream ss;
	ss << "/Timestep_" << std::setfill('0') << std::setw(6) << timestep;
	string dataset_name = ss.str();


	this->loadCalendar(file, dataset_name, sim);
	this->loadPersonTDData(file, dataset_name, sim);

	// Sort the population by id first, in order to increase the speed of cluster reordening
	auto sortByID = [](const Simulator::PersonType& lhs, const Simulator::PersonType& rhs) { return lhs.getId() < rhs.getId(); };
	std::sort(sim->m_population->m_original.begin(), sim->m_population->m_original.end(), sortByID);

	this->loadClusters(file, ss.str() + "/household_clusters", sim->m_households, sim.get()->m_population);
	this->loadClusters(file, ss.str() + "/school_clusters", sim->m_school_clusters, sim.get()->m_population);
	this->loadClusters(file, ss.str() + "/work_clusters", sim->m_work_clusters, sim.get()->m_population);
	this->loadClusters(file, ss.str() + "/primary_community_clusters", sim->m_primary_community, sim.get()->m_population);
	this->loadClusters(file, ss.str() + "/secondary_community_clusters", sim->m_secondary_community, sim.get()->m_population);

	this->updateClusterImmuneIndices(sim);

	if (sim->m_rng != nullptr) {
		this->loadRngState(file, dataset_name, sim);
	}

	file.close();
}


unsigned int Loader::getLastSavedTimestep() const {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	DataSet dataset = DataSet(file.openDataSet("last_timestep"));
	unsigned int data[1];
	dataset.read(data, PredType::NATIVE_UINT);
	dataset.close();
	file.close();
	return data[0];
}


void Loader::setupPopulation(std::shared_ptr<Simulator> sim) const {
	const auto seed = m_pt_config.get<double>("run.rng_seed");
	Random rng(seed);

	sim = SimulatorBuilder::build(m_pt_config, m_pt_disease, m_pt_contact,
								  m_num_threads, m_track_index_case);
}


void Loader::updateClusterImmuneIndices(std::shared_ptr<Simulator> sim) const {
	for (auto cluster : sim->m_households) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_school_clusters) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_work_clusters) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_primary_community) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_secondary_community) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
}

void Loader::loadCalendar(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	DataSet dataset = DataSet(file.openDataSet(dataset_name + "/Calendar"));
	CalendarDataType calendar[1];
	dataset.read(calendar, CalendarDataType::getCompType());
	dataset.close();

	sim->m_calendar->m_day = calendar[0].day;
	sim->m_calendar->m_date = boost::gregorian::from_simple_string(calendar[0].date);
}


void Loader::loadPersonTDData(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	DataSet dataset = DataSet(file.openDataSet(dataset_name + "/PersonTD"));
	unsigned long dims[1] = {sim->m_population.get()->m_original.size()};
	CompType type_person_TD = PersonTDDataType::getCompType();

	for (unsigned int i = 0; i < dims[0]; i++) {
		PersonTDDataType person[1];
		hsize_t dim_sub[1] = {1};

		DataSpace memspace(1, dim_sub, NULL);

		hsize_t offset[1] = {i};
		hsize_t count[1] = {1};
		hsize_t stride[1] = {1};
		hsize_t block[1] = {1};

		DataSpace dataspace = dataset.getSpace();
		dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
		dataset.read(person, type_person_TD, memspace, dataspace);
		sim->m_population->m_original.at(i).m_is_participant = person[0].participant;
		sim->m_population->m_original.at(i).m_health.m_status = HealthStatus(person[0].health_status);
		sim->m_population->m_original.at(i).m_health.m_disease_counter = person[0].disease_counter;
		memspace.close();
		dataspace.close();
	}
	dataset.close();
}


void Loader::loadRngState(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	DataSet dataset = DataSet(file.openDataSet(dataset_name + "/randomgen"));
	std::string rng_state;
	dataset.read(rng_state, StrType(0, H5T_VARIABLE));
	dataset.close();

	sim->m_rng->setState(rng_state);
}


void Loader::loadClusters(H5File& file, std::string full_dataset_name, std::vector<Cluster>& cluster, std::shared_ptr<Population> pop) const {
	DataSet dataset = DataSet(file.openDataSet(full_dataset_name));
	DataSpace dataspace = dataset.getSpace();
	hsize_t dims_clusters[1];
	dataspace.getSimpleExtentDims(dims_clusters, NULL);
	const unsigned int amtIds = dims_clusters[0];
	dataspace.close();

	unsigned int cluster_data[amtIds];
	unsigned int index = 0;

	dataset.read(cluster_data, PredType::NATIVE_UINT);
	dataset.close();

	for(unsigned int i = 0; i < cluster.size(); i++) {
		for(unsigned int j = 0; j < cluster.at(i).getSize(); j++) {
			unsigned int id = cluster_data[index++];
			Simulator::PersonType* person = &pop->m_original.at(id);
			cluster.at(i).m_members.at(j).first = person;
		}
	}
}

void Loader::extractConfigs(string filename) {
	// Check for the existence/validity of the hdf5 file.
	bool hdf5_file_exists = exists(system_complete(filename));
	if (!hdf5_file_exists) {
		throw runtime_error(string(__func__) + "> Hdf5 file " +
			system_complete(filename).string() + " does not exist.");
	}
	const auto file_path_hdf5 = canonical(system_complete(filename));
	if (!is_regular_file(file_path_hdf5)) {
		throw runtime_error(string(__func__) + "> Hdf5 file " +
			system_complete(filename).string() + " is not a regular file.");
	}

	H5File file(filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);

	DataSet dataset = DataSet(file.openDataSet("configuration/configuration"));
	ConfDataType configData[1];
	dataset.read(configData, ConfDataType::getCompType());
	dataset.close();
	file.close();

	auto writeToFile = [](string filename, string content) {
		std::ofstream file;
		file.open(filename.c_str());
		file << content;
		file.close();
	};

	writeToFile(filename + "_config.xml", configData[0].conf_content);
	writeToFile(filename + "_disease.xml", configData[0].disease_content);
	writeToFile(filename + "_contact.xml", configData[0].age_contact_content);
	writeToFile(filename + "_holidays.json", configData[0].holidays_content);
}


}
