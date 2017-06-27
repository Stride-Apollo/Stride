/**
 * @file
 * Source file for the Saver class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include "Hdf5Saver.h"
#include "util/InstallDirs.h"
#include "calendar/Calendar.h"
#include "pop/Population.h"
#include "pop/Person.h"
#include "pop/Traveller.h"
#include "core/Cluster.h"
#include "util/etc.h"
#include "util/SimplePlanner.h"
#include "checkpointing/datatypes/CalendarDataType.h"
#include "checkpointing/datatypes/ConfigDataType.h"
#include "checkpointing/datatypes/PersonTDDataType.h"
#include "checkpointing/datatypes/PersonTIDataType.h"
#include "checkpointing/datatypes/TravellerDataType.h"

#include <vector>

using namespace H5;
using namespace stride::util;
using namespace boost::filesystem;
using std::string;
using std::ostringstream;

namespace stride {

Hdf5Saver::Hdf5Saver(string filename, const ptree& pt_config, int frequency, RunMode run_mode, int start_timestep)
		: m_filename(filename), m_frequency(frequency),
		  m_current_step(start_timestep - 1), m_timestep(start_timestep),
		  m_save_count(0) {

	// Check if the simulator is run in extend mode and not from timestep 0
	if (start_timestep != 0 && run_mode == RunMode::Extend) {
		// If the hdf5 file already exists, append the data, otherwise still run the whole constructor
		if (exists(system_complete(string(filename)))) {

			// Adjust the amount of saved timesteps
			H5File file(m_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
			DataSet dataset = DataSet(file.openDataSet("amt_timesteps"));
			unsigned int data[1];
			dataset.read(data, PredType::NATIVE_UINT);
			dataset.close();
			file.close();

			m_save_count = data[0];
			return;
		}
	}
	try {
		H5File file(m_filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		this->saveConfigs(file, pt_config);
		this->saveTimestepMetadata(file, 0, 0, true);

		file.close();
	} catch (FileIException error) {
		error.printError();
	}
}

void Hdf5Saver::update(const Simulator& sim) {
	m_current_step++;
	if (m_frequency != 0 && m_current_step % m_frequency == 0) {
		this->saveTimestep(sim);
	}
}

void Hdf5Saver::forceSave(const Simulator& sim, int timestep) {
	m_current_step++;

	if (timestep != -1) {
		m_timestep = timestep;
	}
	this->saveTimestep(sim);
}


void Hdf5Saver::saveTimestep(const Simulator& sim) {
	try {
		m_save_count++;
		H5File file(m_filename.c_str(), H5F_ACC_RDWR);

		if (m_current_step == 0) {
			this->savePersonTIData(file, sim);
		}
		stringstream ss;
		ss << "/Timestep_" << std::setfill('0') << std::setw(6) << m_timestep;
		Group group(file.createGroup(ss.str()));


		if (sim.m_rng != nullptr) {
			saveRngState(group, sim);
		}
		this->saveCalendar(group, sim);
		this->savePersonTDData(group, sim);

		this->saveTravellers(group, sim);

		this->saveClusters(group, "household_clusters", sim.m_households);
		this->saveClusters(group, "school_clusters", sim.m_school_clusters);
		this->saveClusters(group, "work_clusters", sim.m_work_clusters);
		this->saveClusters(group, "primary_community_clusters", sim.m_primary_community);
		this->saveClusters(group, "secondary_community_clusters", sim.m_secondary_community);


		this->saveTimestepMetadata(file, m_save_count, m_current_step);
		m_timestep += m_frequency;
		file.close();
	} catch (GroupIException& error) {
		error.printError();
		return;
	} catch (AttributeIException& error) {
		error.printError();
		return;
	} catch (FileIException& error) {
		std::cout << "Trying to open file: " << m_filename << " but failed." << std::endl;
		error.printError();
		return;
	} catch (DataSetIException& error) {
		error.printError();
		return;
	} catch (DataSpaceIException& error) {
		std::cout << "Error while interacting with a dataspace." << std::endl;
		error.printError();
		return;
	} catch (Exception& error) {
		std::cout << "Unknown exception?" << std::endl;
		error.printError();
	}
	return;
}

void Hdf5Saver::saveClusters(Group& group, string dataset_name, const vector<Cluster>& clusters) const {
	auto getAmtIds = [&]() {
		unsigned int amt = 0;
		for (unsigned int i = 0; i < clusters.size(); i++) amt += clusters.at(i).getSize();
		return amt;
	};
	unsigned int amtIds = getAmtIds();

	const unsigned int ndims_clusters = 1;
	hsize_t dims_clusters[ndims_clusters] {amtIds};
	DataSpace dataspace_clusters = DataSpace(ndims_clusters, dims_clusters);
	DataSet dataset_clusters = DataSet(
			group.createDataSet(H5std_string(dataset_name), PredType::NATIVE_UINT, dataspace_clusters));

	auto cluster_data = make_unique<vector<unsigned int>>(amtIds);
	unsigned int index = 0;
	for (unsigned int i = 0; i < clusters.size(); i++) {
		for (unsigned int j = 0; j < clusters.at(i).getSize(); j++) {
			(*cluster_data)[index++] = clusters.at(i).m_members.at(j).first->m_id;
		}
	}
	dataset_clusters.write(cluster_data->data(), PredType::NATIVE_UINT);
	dataspace_clusters.close();
	dataset_clusters.close();
}


void Hdf5Saver::savePersonTIData(H5File& file, const Simulator& sim) const {
	hsize_t dims[1] {sim.getPopulation().get()->m_original.size()};
	DataSpace dataspace = DataSpace(1, dims);

	CompType type_person_TI = PersonTIDataType::getCompType();
	DataSet dataset = DataSet(file.createDataSet("person_time_independent", type_person_TI, dataspace));

	// Persons are saved per chunk
	unsigned int person_index = 0;
	hsize_t chunk_dims[1] = {40000};
	while (person_index < dims[0]) {
		hsize_t selected_dims[1];
		if (person_index + chunk_dims[0] < dims[0]) {
			selected_dims[0] = chunk_dims[0];
		} else {
			selected_dims[0] = dims[0] - person_index;
		}

		PersonTIDataType personData[selected_dims[0]];
		for (unsigned int j = 0; j < selected_dims[0]; j++) {
			#define setAttributePerson(attr_lhs, attr_rhs) personData[j].attr_lhs = sim.getPopulation().get()->m_original.at(person_index).attr_rhs
			setAttributePerson(m_id, m_id);
			setAttributePerson(m_age, m_age);
			setAttributePerson(m_gender, m_gender);
			setAttributePerson(m_household_id, m_household_id);
			setAttributePerson(m_school_id, m_school_id);
			setAttributePerson(m_work_id, m_work_id);
			setAttributePerson(m_prim_comm_id, m_primary_community_id);
			setAttributePerson(m_sec_comm_id, m_secondary_community_id);
			setAttributePerson(m_start_infectiousness, m_health.getStartInfectiousness());
			setAttributePerson(m_start_symptomatic, m_health.getStartSymptomatic());
			personData[j].m_time_infectiousness =
					sim.getPopulation().get()->m_original.at(person_index).m_health.getEndInfectiousness() -
					sim.getPopulation().get()->m_original.at(person_index).m_health.getStartInfectiousness();
			personData[j].m_time_symptomatic =
					sim.getPopulation().get()->m_original.at(person_index).m_health.getEndSymptomatic() -
					sim.getPopulation().get()->m_original.at(person_index).m_health.getStartSymptomatic();
			person_index++;
		}

		// Select a hyperslab in the dataset.
		DataSpace filespace = DataSpace(dataset.getSpace());
		hsize_t offset[1] {person_index - selected_dims[0]};
		filespace.selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

		// Define memory space.
		DataSpace memspace = DataSpace(1, selected_dims, NULL);

		// Write data to the selected portion of the dataset.
		dataset.write(personData, type_person_TI, memspace, filespace);
	}
	#undef setAttributePerson
}


void Hdf5Saver::savePersonTDData(Group& group, const Simulator& sim) const {
	hsize_t dims[1] {sim.getPopulation().get()->m_original.size()};
	CompType type_person_TD = PersonTDDataType::getCompType();
	const unsigned int CHUNK_SIZE = 10000;

	// Dataspace can fit all persons but is chunked in parts of 10000 persons
	DataSpace dataspace = DataSpace(1, dims);
	DSetCreatPropList plist = DSetCreatPropList();
	hsize_t chunk_dims[1] = {CHUNK_SIZE};
	plist.setChunk(1, chunk_dims);

	DataSet dataset;
	if (dims[0] > CHUNK_SIZE) {
		dataset = DataSet(group.createDataSet("person_time_dependent", type_person_TD, dataspace, plist));
	} else {
		dataset = DataSet(group.createDataSet("person_time_dependent", type_person_TD, dataspace));
	}

	using PersonType = Simulator::PersonType;
	const std::vector<PersonType>& population = sim.getPopulation()->m_original;


	// Persons are saved per chunk
	unsigned int person_index = 0;
	while (person_index < dims[0]) {
		hsize_t selected_dims[1];
		if (person_index + chunk_dims[0] < dims[0]) {
			selected_dims[0] = chunk_dims[0];
		} else {
			selected_dims[0] = dims[0] - person_index;
		}

		PersonTDDataType person_data[selected_dims[0]];
		for (unsigned int j = 0; j < selected_dims[0]; j++) {
			const PersonType& person = population[person_index];
			person_data[j].m_participant = person.m_is_participant;
			person_data[j].m_health_status = (unsigned int) person.m_health.getHealthStatus();
			person_data[j].m_disease_counter = (unsigned int) person.m_health.getDiseaseCounter();
			person_data[j].m_on_vacation = person.m_is_on_vacation;
			person_index++;
		}

		// Select a hyperslab in the dataset.
		DataSpace filespace = DataSpace(dataset.getSpace());
		hsize_t offset[1] = {person_index - selected_dims[0]};
		filespace.selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

		// Define memory space.
		DataSpace memspace = DataSpace(1, selected_dims, NULL);

		// Write data to the selected portion of the dataset.
		dataset.write(person_data, type_person_TD, memspace, filespace);
		memspace.close();
		filespace.close();
	}

	dataset.close();
	dataspace.close();
}


void Hdf5Saver::saveTravellers(Group& group, const Simulator& sim) const {

	using PersonType = Simulator::PersonType;
	using Block = SimplePlanner<Simulator::TravellerType>::Block;
	using Agenda = SimplePlanner<Simulator::TravellerType>::Agenda;

	const Agenda& travellers = sim.m_planner.getAgenda();
	hsize_t dims[1] {sim.m_planner.size()};
	CompType type_traveller = TravellerDataType::getCompType();

	DataSpace dataspace = DataSpace(1, dims);
	DataSet dataset = DataSet(group.createDataSet("travellers", type_traveller, dataspace));
	auto traveller_data = make_unique<std::vector<TravellerDataType>>(dims[0]);


	// Obtain the travellers in a single vector, more convenient for index calculations
	vector<Simulator::PersonType*> travellers_seq;
	for (auto&& day : sim.m_planner.getAgenda()) {
		for (auto&& traveller : *(day)) {
			travellers_seq.push_back(traveller->getNewPerson());
		}
	}

	unsigned int current_index = 0;
	unsigned int list_index = 0;

	vector<pair<string,string>> sim_names(travellers_seq.size());

	for (auto&& day : travellers) {
		const Block& current_day = *(day);
		for (auto&& person: current_day) {
			TravellerDataType traveller;

			sim_names.at(current_index) = make_pair(person->getHomeSimulatorId(), person->getDestinationSimulatorId());

			traveller.m_days_left = list_index;
			traveller.m_home_sim_name = sim_names.at(current_index).first.c_str();
			traveller.m_dest_sim_name = sim_names.at(current_index).second.c_str();
			traveller.m_home_sim_index = person->getHomeSimulatorIndex();
			traveller.m_dest_sim_index = sim.m_population->m_original.size() +
										 (std::find(travellers_seq.begin(), travellers_seq.end(),
													person->getNewPerson()) - travellers_seq.begin());

			PersonType original_person = person->getHomePerson();
			#define setAttributeTraveller(attr_lhs, attr_rhs) traveller.attr_lhs = original_person.attr_rhs
			setAttributeTraveller(m_orig_id, m_id);
			setAttributeTraveller(m_age, m_age);
			setAttributeTraveller(m_gender, m_gender);
			setAttributeTraveller(m_orig_household_id, m_household_id);
			setAttributeTraveller(m_orig_school_id, m_school_id);
			setAttributeTraveller(m_orig_work_id, m_work_id);
			setAttributeTraveller(m_orig_prim_comm_id, m_primary_community_id);
			setAttributeTraveller(m_orig_sec_comm_id, m_secondary_community_id);
			setAttributeTraveller(m_start_infectiousness, m_health.getStartInfectiousness());
			setAttributeTraveller(m_start_symptomatic, m_health.getStartSymptomatic());
			traveller.m_time_infectiousness = original_person.m_health.getEndInfectiousness() -
											  original_person.m_health.getStartInfectiousness();
			traveller.m_time_symptomatic = original_person.m_health.getEndSymptomatic() -
										   original_person.m_health.getStartSymptomatic();

			PersonType current_person = *person->getNewPerson();
			traveller.m_participant = current_person.m_is_participant;
			traveller.m_health_status = (unsigned int) current_person.m_health.getHealthStatus();;
			traveller.m_disease_counter = (unsigned int) current_person.m_health.getDiseaseCounter();;
			traveller.m_new_id = current_person.m_id;
			traveller.m_new_household_id = current_person.m_household_id;
			traveller.m_new_school_id = current_person.m_school_id;
			traveller.m_new_work_id = current_person.m_work_id;
			traveller.m_new_prim_comm_id = current_person.m_primary_community_id;
			traveller.m_new_sec_comm_id = current_person.m_secondary_community_id;

			(*traveller_data)[current_index++] = traveller;
		}
		list_index++;
	}
	#undef setAttributeTraveller

	if (sim.m_planner.size() != 0)
		dataset.write(traveller_data->data(), TravellerDataType::getCompType());
	dataset.close();
	dataspace.close();
}


void Hdf5Saver::saveTimestepMetadata(H5File& file, unsigned int total_amt, unsigned int current, bool create) const {
	DataSet dataset_amt;
	if (create == true) {
		hsize_t dims[1] {1};
		DataSpace dataspace = DataSpace(1, dims);
		dataset_amt = file.createDataSet("amt_timesteps", PredType::NATIVE_UINT, dataspace);
	} else {
		dataset_amt = file.openDataSet("amt_timesteps");
	}
	unsigned int amt_timesteps[1] {total_amt};
	dataset_amt.write(amt_timesteps, PredType::NATIVE_UINT);
	dataset_amt.close();


	DataSet dataset_last;
	if (create == true) {
		hsize_t dims[1] {1};
		DataSpace dataspace = DataSpace(1, dims);
		dataset_last = file.createDataSet("last_timestep", PredType::NATIVE_UINT, dataspace);
	} else {
		dataset_last = file.openDataSet("last_timestep");
	}
	unsigned int last_timestep[1] {current};
	dataset_last.write(last_timestep, PredType::NATIVE_UINT);
	dataset_last.close();
}


void Hdf5Saver::saveRngState(Group& group, const Simulator& sim) const {
	hsize_t dims[1] {1};
	DataSpace dataspace = DataSpace(1, dims);
	DataSet dataset = DataSet(group.createDataSet("randomgen", StrType(0, H5T_VARIABLE), dataspace));

	stringstream ss;
	ss << *sim.m_rng;
	string cppString = ss.str();
	const char* rng_state[1] {cppString.c_str()};

	dataset.write(rng_state, StrType(0, H5T_VARIABLE));
	dataset.close();
	dataspace.close();
}


void Hdf5Saver::saveCalendar(Group& group, const Simulator& sim) const {
	hsize_t dims[1] {1};
	CompType typeCalendar = CalendarDataType::getCompType();
	DataSpace dataspace = DataSpace(1, dims);
	DataSet dataset = DataSet(group.createDataSet("calendar", typeCalendar, dataspace));

	stringstream ss;
	ss << sim.m_calendar->getYear() << "-" << sim.m_calendar->getMonth() << "-" << sim.m_calendar->getDay();
	string save_date = ss.str();

	CalendarDataType calendar[1];
	calendar[0].m_day = sim.m_calendar->getSimulationDay();
	calendar[0].m_date = save_date.c_str();
	dataset.write(calendar, typeCalendar);

	dataset.close();
	dataspace.close();
}


void Hdf5Saver::saveConfigs(H5File& file, const ptree& pt_config) const {
	hsize_t dims[1] {1};
	Group group(file.createGroup("/Configuration"));
	DataSpace dataspace = DataSpace(1, dims);

	CompType type_conf_data = ConfigDataType::getCompType();
	DataSet dataset = DataSet(group.createDataSet(H5std_string("configuration"), type_conf_data, dataspace));
	ConfigDataType configData[1];

	auto getStringXmlPtree = [](const ptree xml) {
		ostringstream oss;
		boost::property_tree::xml_parser::write_xml(oss, xml);
		return oss.str();
	};

	// Searches file in DataDir
	auto getPtreeXmlFile = [](string filename) {
		const auto filepath {InstallDirs::getDataDir() /= filename};
		if (!is_regular_file(filepath))
			throw std::runtime_error(string(__func__) + "> File " + filepath.string() + " not present/regular.");
		ptree tree;
		xml_parser::read_xml(filepath.string(), tree, boost::property_tree::xml_parser::trim_whitespace);
		return tree;
	};

	string content_config = getStringXmlPtree(pt_config);
	configData[0].m_config_content = content_config.c_str();
	string content_disease = getStringXmlPtree(getPtreeXmlFile(pt_config.get<string>("run.disease.config")));
	configData[0].m_disease_content = content_disease.c_str();
	string content_age = getStringXmlPtree(getPtreeXmlFile(pt_config.get<string>("run.age_contact_matrix_file")));
	configData[0].m_age_contact_content = content_age.c_str();


	ptree json_tree;
	string filename = pt_config.get<string>("run.holidays");
	const auto filepath {InstallDirs::getDataDir() /= filename};
	if (!is_regular_file(filepath))
		throw std::runtime_error(string(__func__) + "> File " + filepath.string() + " not present/regular.");
	json_parser::read_json(filepath.string(), json_tree);

	ostringstream oss;
	json_parser::write_json(oss, json_tree);
	string content_holidays = oss.str();
	configData[0].m_holidays_content = content_holidays.c_str();


	dataset.write(configData, type_conf_data);
	dataset.close();
	dataspace.close();
	group.close();
}


}
