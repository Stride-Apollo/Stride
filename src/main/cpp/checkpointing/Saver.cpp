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
		Exception::dontPrint();
		H5File file(m_filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		hsize_t dims[1];
		dims[0] = 1;
		Group group(file.createGroup("/configuration"));

		DataSpace dataspace = DataSpace(1, dims);

		CompType type_conf_data = ConfDataType::getCompType();
		DataSet dataset = DataSet(group.createDataSet(H5std_string("configuration"), type_conf_data, dataspace));

		std::ostringstream oss;
		boost::property_tree::xml_parser::write_xml(oss, pt_config);
		string conf_xml = oss.str();
		oss.clear();
		ConfDataType configData[1];
		configData[0].conf_content = conf_xml.c_str();


		const auto file_name_d {m_pt_config.get<string>("run.disease_config_file")};
		const auto file_path_d {InstallDirs::getDataDir() /= file_name_d};
		if (!is_regular_file(file_path_d)) {
			throw std::runtime_error(string(__func__) + "> No file " + file_path_d.string());
		}
		ptree temp;
		xml_parser::read_xml(file_path_d.string(), temp);
		xml_parser::write_xml(oss, temp);
		string xml = oss.str();
		oss.str("");
		configData[0].disease_content = xml.c_str();


		const auto file_name_c {m_pt_config.get<string>("run.holidays_file")};
		const auto file_path_c {InstallDirs::getDataDir() /= file_name_c};
		if (!is_regular_file(file_path_c)) {
			throw std::runtime_error(string(__func__) + "> No file " + file_path_c.string());
		}
		json_parser::read_json(file_path_c.string(), temp);
		json_parser::write_json(oss, temp);
		string json = oss.str();
		oss.str("");
		configData[0].holidays_content = json.c_str();


		const auto file_name_a {m_pt_config.get<string>("run.age_contact_matrix_file")};
		const auto file_path_a {InstallDirs::getDataDir() /= file_name_a};
		if (!is_regular_file(file_path_a)) {
			throw std::runtime_error(string(__func__) + "> No file " + file_path_a.string());
		}
		xml_parser::read_xml(file_path_a.string(), temp);
		xml_parser::write_xml(oss, temp);
		string age_xml = oss.str();
		oss.str("");
		configData[0].age_contact_content = age_xml.c_str();

		dataset.write(configData, type_conf_data);

		dataspace = DataSpace(1, dims);
		unsigned int amt_time[1] = {0};
		dataset = DataSet(file.createDataSet("amt_timesteps", PredType::NATIVE_UINT, dataspace));
		dataset.write(amt_time, PredType::NATIVE_UINT);
		dataset.close();
		dataspace.close();


		dataspace = DataSpace(1, dims);
		unsigned int last_timestep[1] = {0};
		dataset = DataSet(file.createDataSet("last_timestep", PredType::NATIVE_UINT, dataspace));
		dataset.write(last_timestep, PredType::NATIVE_UINT);
		dataset.close();
		dataspace.close();


		dataspace = DataSpace(1, dims);
		dataset = DataSet(file.createDataSet("track_index_case", PredType::NATIVE_INT, dataspace));
		bool track[1] = {track_index_case};
		dataset.write(track, PredType::NATIVE_INT);
		dataset.close();
		dataspace.close();
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
			// Save Person Time Independent
			hsize_t dims[1] = {sim.getPopulation().get()->m_original.size()};
			CompType type_person_TI = PersonTIDataType::getCompType();

			DataSpace dataspace = DataSpace(1, dims);
			DataSet dataset = DataSet(file.createDataSet("personsTI", type_person_TI, dataspace));

			// Persons are saved per chunk
			unsigned int i = 0;
			hsize_t chunk_dims[1] = {40000};
			while (i < dims[0]) {
				hsize_t selected_dims[1];
				if (i + chunk_dims[0] < dims[0]) {
					selected_dims[0] = chunk_dims[0];
				} else {
					selected_dims[0] = dims[0] - i;
				}

				PersonTIDataType personData[selected_dims[0]];
				for (unsigned int j = 0; j < selected_dims[0]; j++) {
					personData[j].ID = sim.getPopulation().get()->m_original.at(i).m_id;
					personData[j].age = sim.getPopulation().get()->m_original.at(i).m_age;
					personData[j].gender = sim.getPopulation().get()->m_original.at(i).m_gender;
					personData[j].household_ID = sim.getPopulation().get()->m_original.at(i).m_household_id;
					personData[j].school_ID = sim.getPopulation().get()->m_original.at(i).m_school_id;
					personData[j].work_ID = sim.getPopulation().get()->m_original.at(i).m_work_id;
					personData[j].prim_comm_ID = sim.getPopulation().get()->m_original.at(i).m_primary_community_id;
					personData[j].sec_comm_ID = sim.getPopulation().get()->m_original.at(i).m_secondary_community_id;
					personData[j].start_infectiousness = sim.getPopulation().get()->m_original.at(i).m_health.getStartInfectiousness();
					personData[j].start_symptomatic = sim.getPopulation().get()->m_original.at(i).m_health.getStartSymptomatic();
					personData[j].time_infectiousness =
							sim.getPopulation().get()->m_original.at(i).m_health.getEndInfectiousness() - sim.getPopulation().get()->m_original.at(i).m_health.getStartInfectiousness();
					personData[j].time_symptomatic =
							sim.getPopulation().get()->m_original.at(i).m_health.getEndSymptomatic() - sim.getPopulation().get()->m_original.at(i).m_health.getStartSymptomatic();
					i++;
				}

				// Select a hyperslab in the dataset.
				DataSpace filespace = DataSpace(dataset.getSpace());
				hsize_t offset[1] = {i-selected_dims[0]};
				filespace.selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

				// Define memory space.
				DataSpace memspace = DataSpace(1, selected_dims, NULL);

				// Write data to the selected portion of the dataset.
				dataset.write(personData, type_person_TI, memspace, filespace);
			}
		}



		// Save amt_timesteps
		DataSet timeset = file.openDataSet("amt_timesteps");
		unsigned int amt_timesteps[1] = {m_save_count};
		timeset.write(amt_timesteps, PredType::NATIVE_UINT);
		timeset.close();

		// Save the last timestep (current one that is being saved)
		DataSet last_timestepset = file.openDataSet("last_timestep");
		unsigned int last_timestep[1] = {m_timestep};
		last_timestepset.write(last_timestep, PredType::NATIVE_UINT);
		last_timestepset.close();

		stringstream ss;
		ss << "/Timestep_" << m_timestep;

		Group group(file.createGroup(ss.str()));


		auto saveRngState = [](Group& group, const Simulator& sim) {
			hsize_t dims[1] = {1};
			DataSpace dataspace = DataSpace(1, dims);

			StrType str_type(0, H5T_VARIABLE);
			DataSet dataset = DataSet(group.createDataSet("randomgen", str_type, dataspace));
			stringstream ss;
			ss << *sim.m_rng;
			string rng_state[1] = {ss.str()};
			dataset.write(rng_state, str_type);
			dataset.close();
		};

		if (sim.m_rng != nullptr) {
			saveRngState(group, sim);
		}



		// Save Calendar

		hsize_t dims[1];
		dims[0] = 1;
		CompType typeCalendar = CalendarDataType::getCompType();
		DataSpace dataspace = DataSpace(1, dims);
		DataSet dataset = DataSet(group.createDataSet("Calendar", typeCalendar, dataspace));

		CalendarDataType calendar[1];
		calendar[0].day = sim.m_calendar->getSimulationDay();
		ss.str("");
		ss.clear();
		ss << sim.m_calendar->getYear() << "-" << sim.m_calendar->getMonth() << "-" << sim.m_calendar->getDay();
		string save_date = ss.str();
		calendar[0].date = save_date.c_str();
		dataset.write(calendar, typeCalendar);


		// Save Traveller Person Data

		// dims[0] = sim.getPopulation().get()->m_visitors.size();
		// CompType typeTravellerData(sizeof(TravellerDataType));

		// =============
		// Save clusters
		// =============

		//   Household clusters
		this->saveClusters(group, "household_clusters", sim.m_households);
		//   School clusters
		this->saveClusters(group, "school_clusters", sim.m_school_clusters);
		//   Work clusters
		this->saveClusters(group, "work_clusters", sim.m_work_clusters);
		//   Primary Community clusters
		this->saveClusters(group, "primary_community_clusters", sim.m_primary_community);
		//   Secondary Community clusters
		this->saveClusters(group, "secondary_community_clusters", sim.m_secondary_community);

		// Save Person Time Dependent
		dims[0] = sim.getPopulation().get()->m_original.size();
		CompType type_person_TD = PersonTDDataType::getCompType();

		// Dataspace can fit all persons but is chunked in parts of 100 persons
		dataspace = DataSpace(1, dims);
		DSetCreatPropList plist = DSetCreatPropList();
		hsize_t chunk_dims[1] = {40000};
		plist.setChunk(1, chunk_dims);
		dataset = DataSet(group.createDataSet("PersonTD", type_person_TD, dataspace, plist));

		// Persons are saved per chunk
		unsigned int i = 0;
		while (i < dims[0]) {
			hsize_t selected_dims[1];
			if (i + chunk_dims[0] < dims[0]) {
				selected_dims[0] = chunk_dims[0];
			} else {
				selected_dims[0] = dims[0] - i;
			}

			PersonTDDataType person_data[selected_dims[0]];
			for (unsigned int j = 0; j < selected_dims[0]; j++) {
				person_data[j].at_household = sim.getPopulation().get()->m_original.at(i).m_at_household;
				person_data[j].at_school = sim.getPopulation().get()->m_original.at(i).m_at_school;
				person_data[j].at_work = sim.getPopulation().get()->m_original.at(i).m_at_work;
				person_data[j].at_prim_comm = sim.getPopulation().get()->m_original.at(i).m_at_primary_community;
				person_data[j].at_sec_comm = sim.getPopulation().get()->m_original.at(i).m_at_secondary_community;
				person_data[j].participant = sim.getPopulation().get()->m_original.at(i).m_is_participant;
				person_data[j].health_status = (unsigned int) sim.getPopulation().get()->m_original.at(i).m_health.getHealthStatus();
				person_data[j].disease_counter = (unsigned int) sim.getPopulation().get()->m_original.at(i).m_health.getDiseaseCounter();
				i++;
			}

			// Select a hyperslab in the dataset.
			DataSpace filespace = DataSpace(dataset.getSpace());
			hsize_t offset[1] = {i-selected_dims[0]};
			filespace.selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

			// Define memory space.
			DataSpace memspace = DataSpace(1, selected_dims, NULL);

			// Write data to the selected portion of the dataset.
			dataset.write(person_data, type_person_TD, memspace, filespace);
		}

		dataspace.close();
		dataset.close();
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

void Saver::saveClusters(Group& group, string dataset_name, const vector<Cluster>& clusters) {
	auto getAmtIds = [&]() {
		unsigned int amt = 0;
		for (unsigned int i = 0; i < clusters.size(); i++) amt += clusters.at(i).getSize();
		return amt;
	};
	unsigned int amtIds = getAmtIds();

	const unsigned int ndims_clusters = 1;
	hsize_t dims_clusters[ndims_clusters] = {amtIds};
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


}
