/**
 * @file
 * Source file for the Saver class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include "Saver.h"
#include "util/InstallDirs.h"
#include "calendar/Calendar.h"
#include "pop/Population.h"
#include "core/Cluster.h"
#include "checkpointing/customDataTypes/CalendarDataType.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
// #include "checkpointing/customDataTypes/TravellerDataType.h"

#include <vector>

using namespace H5;
using namespace stride::util;
using namespace boost::filesystem;
using std::string;
using std::ostringstream;

namespace stride {

Saver::Saver(string filename, ptree pt_config, int frequency, bool track_index_case, string simulator_run_mode, int start_timestep)
	: m_filename(filename), m_frequency(frequency),
	  m_pt_config(pt_config), m_current_step(start_timestep - 1),
	  m_timestep(start_timestep), m_save_count(0) {

	// Check if the simulator is run in extend mode and not from timestep 0
	if (start_timestep != 0 && simulator_run_mode == "extend") {
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

		this->saveConfigs(file);
		this->saveTimestepMetadata(file, 0, 0, true);
		this->saveTrackIndexCase(file, track_index_case);

		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}

void Saver::update(const LocalSimulatorAdapter& local_sim) {
	m_current_step++;
	if (m_frequency != 0 && m_current_step%m_frequency == 0) {
		this->saveTimestep(*(local_sim.m_sim));
	}
}

void Saver::forceSave(const LocalSimulatorAdapter& local_sim, int timestep) {
	// m_current_step++;

	if (timestep != -1) {
		m_timestep = timestep;
	}
	this->saveTimestep(*(local_sim.m_sim));
}


void Saver::saveTimestep(const Simulator& sim) {
	try {
		m_save_count++;
		H5File file(m_filename.c_str(), H5F_ACC_RDWR);

		if (m_current_step == 0) {
			this->savePersonTIData(file, sim);
		}

		stringstream ss;
		ss << "/Timestep_" << m_timestep;
		Group group(file.createGroup(ss.str()));

		// Only save when unipar dummy implementation is used, otherwise sim.m_rng == nullptr
		if (sim.m_rng != nullptr) {
			saveRngState(group, sim);
		}

		this->saveCalendar(group, sim);
		this->savePersonTDData(group, sim);

		this->saveClusters(group, "household_clusters", sim.m_households);
		this->saveClusters(group, "school_clusters", sim.m_school_clusters);
		this->saveClusters(group, "work_clusters", sim.m_work_clusters);
		this->saveClusters(group, "primary_community_clusters", sim.m_primary_community);
		this->saveClusters(group, "secondary_community_clusters", sim.m_secondary_community);


		// TODO Save Traveller Person Data
		// dims[0] = sim.getPopulation().get()->m_visitors.size();
		// CompType typeTravellerData(sizeof(TravellerDataType));

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

void Saver::saveClusters(Group& group, string dataset_name, const vector<Cluster>& clusters) const {
	auto getAmtIds = [&]() {
		unsigned int amt = 0;
		for (unsigned int i = 0; i < clusters.size(); i++) amt += clusters.at(i).getSize();
		return amt;
	};
	unsigned int amtIds = getAmtIds();

	const unsigned int ndims_clusters = 1;
	hsize_t dims_clusters[ndims_clusters] {amtIds};
	DataSpace dataspace_clusters = DataSpace(ndims_clusters, dims_clusters);
	DataSet dataset_clusters = DataSet(group.createDataSet(H5std_string(dataset_name), PredType::NATIVE_UINT, dataspace_clusters));

	unsigned int cluster_data[amtIds];
	unsigned int index = 0;
	for (unsigned int i = 0; i < clusters.size(); i++) {
		for (unsigned int j = 0; j < clusters.at(i).getSize(); j++) {
			cluster_data[index++] = clusters.at(i).m_members.at(j).first->m_id;
		}
	}
	dataset_clusters.write(cluster_data, PredType::NATIVE_UINT);
	dataspace_clusters.close();
	dataset_clusters.close();
}


void Saver::savePersonTIData(H5File& file, const Simulator& sim) const {
	hsize_t dims[1] {sim.getPopulation().get()->m_original.size()};
	DataSpace dataspace = DataSpace(1, dims);

	CompType type_person_TI = PersonTIDataType::getCompType();
	DataSet dataset = DataSet(file.createDataSet("personsTI", type_person_TI, dataspace));

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
			setAttributePerson(ID, m_id);
			setAttributePerson(age, m_age);
			setAttributePerson(gender, m_gender);
			setAttributePerson(household_ID, m_household_id);
			setAttributePerson(school_ID, m_school_id);
			setAttributePerson(work_ID, m_work_id);
			setAttributePerson(prim_comm_ID, m_primary_community_id);
			setAttributePerson(sec_comm_ID, m_secondary_community_id);
			setAttributePerson(start_infectiousness, m_health.getStartInfectiousness());
			setAttributePerson(start_symptomatic, m_health.getStartSymptomatic());
			personData[j].time_infectiousness = sim.getPopulation().get()->m_original.at(person_index).m_health.getEndInfectiousness() -
						sim.getPopulation().get()->m_original.at(person_index).m_health.getStartInfectiousness();
			personData[j].time_symptomatic = sim.getPopulation().get()->m_original.at(person_index).m_health.getEndSymptomatic() -
						sim.getPopulation().get()->m_original.at(person_index).m_health.getStartSymptomatic();
			person_index++;
		}

		// Select a hyperslab in the dataset.
		DataSpace filespace = DataSpace(dataset.getSpace());
		hsize_t offset[1] { person_index - selected_dims[0] };
		filespace.selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

		// Define memory space.
		DataSpace memspace = DataSpace(1, selected_dims, NULL);

		// Write data to the selected portion of the dataset.
		dataset.write(personData, type_person_TI, memspace, filespace);
	}
	#undef setAttributePerson
}


void Saver::savePersonTDData(Group& group, const Simulator& sim) const {
	hsize_t dims[1] { sim.getPopulation().get()->m_original.size() };
	CompType type_person_TD = PersonTDDataType::getCompType();

	// Dataspace can fit all persons but is chunked in parts of 100 persons
	DataSpace dataspace = DataSpace(1, dims);
	DSetCreatPropList plist = DSetCreatPropList();
	hsize_t chunk_dims[1] = {40000};
	plist.setChunk(1, chunk_dims);
	DataSet dataset = DataSet(group.createDataSet("PersonTD", type_person_TD, dataspace, plist));

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
			person_data[j].at_household = sim.getPopulation().get()->m_original.at(person_index).m_at_household;
			person_data[j].at_school = sim.getPopulation().get()->m_original.at(person_index).m_at_school;
			person_data[j].at_work = sim.getPopulation().get()->m_original.at(person_index).m_at_work;
			person_data[j].at_prim_comm = sim.getPopulation().get()->m_original.at(person_index).m_at_primary_community;
			person_data[j].at_sec_comm = sim.getPopulation().get()->m_original.at(person_index).m_at_secondary_community;
			person_data[j].participant = sim.getPopulation().get()->m_original.at(person_index).m_is_participant;
			person_data[j].health_status = (unsigned int) sim.getPopulation().get()->m_original.at(person_index).m_health.getHealthStatus();
			person_data[j].disease_counter = (unsigned int) sim.getPopulation().get()->m_original.at(person_index).m_health.getDiseaseCounter();
			person_index++;
		}

		// Select a hyperslab in the dataset.
		DataSpace filespace = DataSpace(dataset.getSpace());
		hsize_t offset[1] = {person_index-selected_dims[0]};
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


void Saver::saveTimestepMetadata(H5File& file, unsigned int total_amt, unsigned int current, bool create) const {
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


void Saver::saveRngState(Group& group, const Simulator& sim) const {
	hsize_t dims[1] {1};
	DataSpace dataspace = DataSpace(1, dims);
	DataSet dataset = DataSet(group.createDataSet("randomgen", StrType(0, H5T_VARIABLE), dataspace));

	stringstream ss;
	ss << *sim.m_rng;
	string rng_state[1] {ss.str()};

	dataset.write(rng_state, StrType(0, H5T_VARIABLE));
	dataset.close();
	dataspace.close();
}


void Saver::saveCalendar(Group& group, const Simulator& sim) const {
	hsize_t dims[1] {1};
	CompType typeCalendar = CalendarDataType::getCompType();
	DataSpace dataspace = DataSpace(1, dims);
	DataSet dataset = DataSet(group.createDataSet("Calendar", typeCalendar, dataspace));

	stringstream ss;
	ss << sim.m_calendar->getYear() << "-" << sim.m_calendar->getMonth() << "-" << sim.m_calendar->getDay();
	string save_date = ss.str();

	CalendarDataType calendar[1];
	calendar[0].day = sim.m_calendar->getSimulationDay();
	calendar[0].date = save_date.c_str();
	dataset.write(calendar, typeCalendar);

	dataset.close();
	dataspace.close();
}


void Saver::saveConfigs(H5File& file) const {
	hsize_t dims[1] {1};
	Group group(file.createGroup("/configuration"));
	DataSpace dataspace = DataSpace(1, dims);

	CompType type_conf_data = ConfDataType::getCompType();
	DataSet dataset = DataSet(group.createDataSet(H5std_string("configuration"), type_conf_data, dataspace));
	ConfDataType configData[1];

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
		xml_parser::read_xml(filepath.string(), tree);
		return tree;
	};

	string content_config = getStringXmlPtree(m_pt_config);
	configData[0].conf_content = content_config.c_str();
	string content_disease = getStringXmlPtree(getPtreeXmlFile(m_pt_config.get<string>("run.disease_config_file")));
	configData[0].disease_content = content_disease.c_str();
	string content_age = getStringXmlPtree(getPtreeXmlFile(m_pt_config.get<string>("run.age_contact_matrix_file")));
	configData[0].age_contact_content = content_age.c_str();


	ptree json_tree;
	string filename = m_pt_config.get<string>("run.holidays_file");
	const auto filepath {InstallDirs::getDataDir() /= filename};
	if (!is_regular_file(filepath))
		throw std::runtime_error(string(__func__) + "> File " + filepath.string() + " not present/regular.");
	json_parser::read_json(filepath.string(), json_tree);

	ostringstream oss;
	json_parser::write_json(oss, json_tree);
	string content_holidays = oss.str();
	configData[0].holidays_content = content_holidays.c_str();


	dataset.write(configData, type_conf_data);
	dataset.close();
	dataspace.close();
	group.close();
}

void Saver::saveTrackIndexCase(H5File& file, bool track_index_case) const {
	hsize_t dims[1] = {1};
	DataSpace dataspace = DataSpace(1, dims);
	DataSet dataset = DataSet(file.createDataSet("track_index_case", PredType::NATIVE_INT, dataspace));
	int track[1] = {track_index_case};
	dataset.write(track, PredType::NATIVE_INT);
	dataset.close();
	dataspace.close();
}


}
