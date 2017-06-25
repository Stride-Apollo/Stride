/**
 * @file
 * Source file for the Loader class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iomanip>
#include "Hdf5Loader.h"
#include "calendar/Calendar.h"
#include "util/InstallDirs.h"
#include "pop/Population.h"
#include "checkpointing/datatypes/CalendarDataType.h"
#include "checkpointing/datatypes/ConfigDataType.h"
#include "checkpointing/datatypes/PersonTDDataType.h"
#include "checkpointing/datatypes/PersonTIDataType.h"
#include "checkpointing/datatypes/TravellerDataType.h"
#include "sim/SimulatorBuilder.h"
#include "pop/PopulationBuilder.h"
#include "core/Cluster.h"
#include "util/etc.h"

#include <algorithm>
#include <vector>
#include <string>

using namespace H5;
using namespace boost::property_tree;
using namespace std;
using namespace boost::filesystem;
using namespace stride::util;

namespace stride {


Hdf5Loader::Hdf5Loader(const char *filename) :
 	m_filename(filename) {

	try {
		this->loadConfigs();
	} catch(FileIException error) {
		error.printError();
	}
}

void Hdf5Loader::loadConfigs() {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);

	DataSet dataset = DataSet(file.openDataSet("Configuration/configuration"));
	ConfigDataType configData[1];
	dataset.read(configData, ConfigDataType::getCompType());
	dataset.close();
	file.close();

	auto getPropTree = [](string xml_content, ptree& dest_pt) {
		istringstream iss;
		iss.str(xml_content);
		xml_parser::read_xml(iss, dest_pt, boost::property_tree::xml_parser::trim_whitespace);
	};

	getPropTree(configData[0].m_config_content, m_pt_config);
	getPropTree(configData[0].m_disease_content, m_pt_disease);
	getPropTree(configData[0].m_age_contact_content, m_pt_contact);
}


void Hdf5Loader::loadFromTimestep(unsigned int timestep, std::shared_ptr<Simulator> sim) const {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);

	stringstream ss;
	ss << "/Timestep_" << std::setfill('0') << std::setw(6) << timestep;
	string dataset_name = ss.str();


	this->loadCalendar(file, dataset_name, sim);
	this->loadPersonTDData(file, dataset_name, sim);
	this->loadTravellers(file, dataset_name, sim);

	// Sort the population by id first, in order to increase the speed of cluster reordening
	auto sortByID = [](const Simulator::PersonType& lhs, const Simulator::PersonType& rhs) { return lhs.getId() < rhs.getId(); };
	std::sort(sim->m_population->m_original.begin(), sim->m_population->m_original.end(), sortByID);

	this->loadClusters(file, dataset_name + "/household_clusters", sim->m_households, sim);
	this->loadClusters(file, dataset_name + "/school_clusters", sim->m_school_clusters, sim);
	this->loadClusters(file, dataset_name + "/work_clusters", sim->m_work_clusters, sim);
	this->loadClusters(file, dataset_name + "/primary_community_clusters", sim->m_primary_community, sim);
	this->loadClusters(file, dataset_name + "/secondary_community_clusters", sim->m_secondary_community, sim);

	this->updateClusterImmuneIndices(sim);

	if (sim->m_rng != nullptr) {
		this->loadRngState(file, dataset_name, sim);
	}

	file.close();
}


unsigned int Hdf5Loader::getLastSavedTimestep() const {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	DataSet dataset = DataSet(file.openDataSet("last_timestep"));
	unsigned int data[1];
	dataset.read(data, PredType::NATIVE_UINT);
	dataset.close();
	file.close();
	return data[0];
}


void Hdf5Loader::updateClusterImmuneIndices(std::shared_ptr<Simulator> sim) const {
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

void Hdf5Loader::loadCalendar(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	DataSet dataset = DataSet(file.openDataSet(dataset_name + "/calendar"));
	CalendarDataType calendar[1];
	dataset.read(calendar, CalendarDataType::getCompType());
	dataset.close();

	sim->m_calendar->m_day = calendar[0].m_day;
	sim->m_calendar->m_date = boost::gregorian::from_simple_string(calendar[0].m_date);
}


void Hdf5Loader::loadTravellers(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	try {
		DataSet dataset = DataSet(file.openDataSet(dataset_name + "/travellers"));
		DataSpace dataspace = dataset.getSpace();
		hsize_t dims_travellers[1];
		dataspace.getSimpleExtentDims(dims_travellers, NULL);
		const unsigned int amt_travellers = dims_travellers[0];
		dataspace.close();

		if (amt_travellers == 0) {
			dataset.close();
			return;
		}

		auto travellers = make_unique<std::vector<TravellerDataType>>(amt_travellers);

		dataset.read(travellers->data(), TravellerDataType::getCompType());

		// TODO if necessary, count the travellers for each day, reserve in the planner

		// The travellers are saved in order of their index in the new simulator
		// Therefore, we can just iterate over them and add them without worrying about changing the original order
		for (auto object : *travellers) {

			// First of all, add the person to the population (visitors)
			Simulator::PersonType person = Simulator::PersonType(
				object.m_new_id, object.m_age,
				object.m_new_household_id, object.m_new_school_id, object.m_new_work_id,
				object.m_new_prim_comm_id, object.m_new_sec_comm_id,
				object.m_start_infectiousness, object.m_start_symptomatic,
				object.m_time_infectiousness, object.m_time_symptomatic
			);
			person.m_health.m_status = HealthStatus(object.m_health_status);
			person.m_health.m_disease_counter = object.m_disease_counter;
			person.m_is_participant = object.m_participant;

			sim->m_population->m_visitors.add(object.m_days_left, person);

			// Secondly, add the traveller to the planner in the simulator

			// Construct the person from the original simulator (his health does not matter, since his new one is used and returned eventually)
			Simulator::PersonType original_person = Simulator::PersonType(
				object.m_orig_id, object.m_age,
				object.m_orig_household_id, object.m_orig_school_id, object.m_orig_work_id,
				object.m_orig_prim_comm_id, object.m_orig_sec_comm_id,
				object.m_start_infectiousness, object.m_start_symptomatic,
				object.m_time_infectiousness, object.m_time_symptomatic
			);

			string home_sim_name = object.m_home_sim_name;
			string dest_sim_name = object.m_dest_sim_name;

			Simulator::TravellerType traveller = Simulator::TravellerType(
				original_person, sim->m_population->m_visitors.getModifiableDay(object.m_days_left)->back().get(),
				home_sim_name, dest_sim_name, object.m_home_sim_index);

			traveller.getNewPerson()->setOnVacation(false);
			sim->m_planner.add(object.m_days_left, traveller);


			// Finally, add the traveller to clusters
			Simulator::PersonType* added_person = sim->m_population->m_visitors.getModifiableDay(object.m_days_left)->back().get();

			sim->m_work_clusters.at(object.m_new_work_id).addPerson(added_person);
			sim->m_primary_community.at(object.m_new_prim_comm_id).addPerson(added_person);
			sim->m_secondary_community.at(object.m_new_sec_comm_id).addPerson(added_person);


			// Since the travellers are ordered, it is safe to set these values every iteration
			sim->m_next_id = object.m_new_id + 1;
			sim->m_next_hh_id = object.m_new_household_id + 1;

		}

	} catch (DataSetIException e) {
		// The dataset does not exist, no traveller information was stored.
		return;
	}
}


void Hdf5Loader::loadPersonTDData(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	DataSet dataset = DataSet(file.openDataSet(dataset_name + "/person_time_dependent"));
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
		sim->m_population->m_original.at(i).m_is_participant = person[0].m_participant;
		sim->m_population->m_original.at(i).m_health.m_status = HealthStatus(person[0].m_health_status);
		sim->m_population->m_original.at(i).m_health.m_disease_counter = person[0].m_disease_counter;
		memspace.close();
		dataspace.close();
	}
	dataset.close();
}


void Hdf5Loader::loadRngState(H5File& file, string dataset_name, shared_ptr<Simulator> sim) const {
	DataSet dataset = DataSet(file.openDataSet(dataset_name + "/randomgen"));
	std::string rng_state;
	dataset.read(rng_state, StrType(0, H5T_VARIABLE));
	dataset.close();

	sim->m_rng->setState(rng_state);
}


void Hdf5Loader::loadClusters(H5File& file, std::string full_dataset_name, std::vector<Cluster>& cluster, std::shared_ptr<Simulator> sim) const {
	std::shared_ptr<Population> pop = sim->m_population;

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

	// Collect all the travellers in a single vector for convenience
	vector<Simulator::PersonType*> travellers;
	for (auto&& day : sim->m_planner.getAgenda()) {
		for (auto&& traveller : *(day)) {
			travellers.push_back(traveller->getNewPerson());
		}
	}

	for(unsigned int i = 0; i < cluster.size(); i++) {
		for(unsigned int j = 0; j < cluster.at(i).getSize(); j++) {
			unsigned int id = cluster_data[index++];

			if (id < pop->m_original.size()) {
				Simulator::PersonType* person = &pop->m_original.at(id);
				cluster.at(i).m_members.at(j).first = person;
			} else {
				// Get the pointer to the person via the travel planner
				auto traveller = std::find_if(
					travellers.begin(),
					travellers.end(),
					[&id](Simulator::PersonType* traveller)->bool {
						return (traveller)->getId() == id;
				});
				cluster.at(i).m_members.at(j).first = (*traveller);
			}

		}
	}
}

void Hdf5Loader::extractConfigs(string filename) {
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

	DataSet dataset = DataSet(file.openDataSet("Configuration/configuration"));
	ConfigDataType configData[1];
	dataset.read(configData, ConfigDataType::getCompType());
	dataset.close();
	file.close();

	auto writeToFile = [](string filename, string content) {
		std::ofstream file;
		file.open(filename.c_str());
		file << content;
		file.close();
	};

	writeToFile(filename + "_config.xml", configData[0].m_config_content);
	writeToFile(filename + "_disease.xml", configData[0].m_disease_content);
	writeToFile(filename + "_contact.xml", configData[0].m_age_contact_content);
	writeToFile(filename + "_holidays.json", configData[0].m_holidays_content);
}


}
