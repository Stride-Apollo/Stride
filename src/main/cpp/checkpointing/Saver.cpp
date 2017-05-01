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
#include "checkpointing/customDataTypes/CalendarDataType.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
#include "checkpointing/customDataTypes/RNGDataType.h"

#include <vector>

using namespace H5;
using namespace stride::util;
using namespace boost::filesystem;

namespace stride {

Saver::Saver(const char* filename, ptree pt_config, int frequency, bool track_index_case, std::string simulator_run_mode, int start_timestep)
	: m_filename(filename), m_frequency(frequency), 
	  m_pt_config(pt_config), m_current_step(start_timestep - 1), 
	  m_timestep(start_timestep), m_save_count(0) {
	
	// Check if the simulator is run in extend mode and not from timestep 0
	if (start_timestep != 0 && simulator_run_mode == "extend") {
		// If the hdf5 file already exists, append the data, otherwise still run the whole constructor
		if (exists(system_complete(std::string(filename)))) {

			// Adjust the amount of saved timesteps
			H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
			DataSet* dataset = new DataSet(file.openDataSet("amt_timesteps"));
			unsigned int data[1];
			dataset->read(data, PredType::NATIVE_UINT);
			dataset->close();
			file.close();

			m_save_count = 	data[0];
			return;
		}
	}
	try {
		Exception::dontPrint();
		H5File file(m_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		hsize_t dims[1];
		dims[0] = 1;
		Group group(file.createGroup("/configuration"));

		DataSpace* dataspace = new DataSpace(1, dims);

		CompType typeConfData(sizeof(ConfDataType));
		StrType tid1(0, H5T_VARIABLE);
		typeConfData.insertMember(H5std_string("conf_content"), HOFFSET(ConfDataType, conf_content), tid1);
		typeConfData.insertMember(H5std_string("disease_content"), HOFFSET(ConfDataType, disease_content), tid1);
		typeConfData.insertMember(H5std_string("age_contact_content"), HOFFSET(ConfDataType, age_contact_content), tid1);
		typeConfData.insertMember(H5std_string("holidays_content"), HOFFSET(ConfDataType, holidays_content), tid1);
		DataSet* dataset = new DataSet(group.createDataSet(H5std_string("configuration"), typeConfData, *dataspace));
		std::ostringstream oss;
		boost::property_tree::xml_parser::write_xml(oss, pt_config);
		std::string conf_xml = oss.str();
		ConfDataType configData[1];

		configData[0].conf_content = conf_xml.c_str();
		const auto file_name_d {m_pt_config.get<std::string>("run.disease_config_file")};
		const auto file_path_d {InstallDirs::getDataDir() /= file_name_d};
		if (!is_regular_file(file_path_d)) {
			throw std::runtime_error(std::string(__func__) + "> No file " + file_path_d.string());
		}
		ptree temp;
		xml_parser::read_xml(file_path_d.string(), temp);
		std::ostringstream oss2;
		xml_parser::write_xml(oss2, temp);
		std::string xml = oss2.str();
		configData[0].disease_content = xml.c_str();
		std::string filepath = pt_config.get<std::string>("run.holidays_file");
		const auto file_name_c {m_pt_config.get<std::string>("run.holidays_file")};
		const auto file_path_c {InstallDirs::getDataDir() /= file_name_c};
		if (!is_regular_file(file_path_c)) {
			throw std::runtime_error(std::string(__func__) + "> No file " + file_path_c.string());
		}

		json_parser::read_json(file_path_c.string(), temp);
		std::ostringstream oss3;
		json_parser::write_json(oss3, temp);
		std::string json = oss3.str();
		configData[0].holidays_content = json.c_str();
		const auto file_name_a {m_pt_config.get<std::string>("run.age_contact_matrix_file")};
		const auto file_path_a {InstallDirs::getDataDir() /= file_name_a};
		if (!is_regular_file(file_path_a)) {
			throw std::runtime_error(std::string(__func__) + "> No file " + file_path_a.string());
		}
		xml_parser::read_xml(file_path_a.string(), temp);
		std::ostringstream oss4;
		xml_parser::write_xml(oss4, temp);
		std::string age_xml = oss4.str();
		configData[0].age_contact_content = age_xml.c_str();
		/*const auto file_name_p {m_pt_config.get<std::string>("run.population_file")};
		const auto file_path_p {InstallDirs::getDataDir() /= file_name_p};
		if (!is_regular_file(file_path_p)) {
			throw std::runtime_error(std::string(__func__) + "> No file " + file_path_p.string());
		}
		xml_parser::read_xml(file_path_p.string(), temp);
		oss.str("");
		xml_parser::write_xml(oss, temp);
		xml = oss.str();
		configData[0].population_content = xml.c_str();

		std::cout << configData[0].holidays_content;*/
		//std::cout << "CONF CONTENT: " << configData[0].conf_content << std::endl;
		//std::cout << "DISEASE CONTENT: " << configData[0].disease_content << std::endl;
		//std::cout << "HOLIDAYS CONTENT: " << configData[0].holidays_content << std::endl;
		//std::cout << "AGE CONTENT: " << configData[0].age_contact_content << std::endl;

		dataset->write(configData, typeConfData);
		delete dataspace;
		delete dataset;

		hsize_t dims2[2];
		// TODO amt_ages
		dims2[0] = 5;
		dims2[1] = 5;
		dataspace = new DataSpace(2, dims2);
		dataset = new DataSet(group.createDataSet("agecontact", PredType::NATIVE_DOUBLE, *dataspace));

		delete dataspace;
		delete dataset;

		dataspace = new DataSpace(1, dims);
		unsigned int amt_time[1] = {0};
		dataset = new DataSet(file.createDataSet("amt_timesteps", PredType::NATIVE_UINT, *dataspace));
		dataset->write(amt_time, PredType::NATIVE_UINT);

		delete dataspace;
		delete dataset;

		dataspace = new DataSpace(1, dims);
		unsigned int last_timestep[1] = {0};
		dataset = new DataSet(file.createDataSet("last_timestep", PredType::NATIVE_UINT, *dataspace));
		dataset->write(last_timestep, PredType::NATIVE_UINT);

		delete dataspace;
		delete dataset;

		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(file.createDataSet("track_index_case", PredType::NATIVE_INT, *dataspace));
		bool track[1] = {track_index_case};
		dataset->write(track, PredType::NATIVE_INT);
		dataset->close();
		dataspace->close();
		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}

void Saver::update(const Simulator& sim) {
	m_current_step++;
	if (m_frequency != 0 && m_current_step%m_frequency == 0) {
		this->saveTimestep(sim);
	}
}

void Saver::forceSave(const Simulator& sim, int timestep) {
	m_current_step++;

	if (timestep != -1) {
		m_timestep = timestep;
	}
	this->saveTimestep(sim);
}


void Saver::saveTimestep(const Simulator& sim) {
	try {
		m_save_count++;

		H5File file(m_filename, H5F_ACC_RDWR);

		if (m_current_step == 0) {
			// Save Person Time Independent
			hsize_t dims[1] = {sim.getPopulation().get()->size()};
			CompType typePersonTI(sizeof(PersonTIDataType));
			typePersonTI.insertMember(H5std_string("ID"), HOFFSET(PersonTIDataType, ID), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("age"), HOFFSET(PersonTIDataType, age), PredType::NATIVE_DOUBLE);
			typePersonTI.insertMember(H5std_string("gender"), HOFFSET(PersonTIDataType, gender), PredType::NATIVE_CHAR);
			typePersonTI.insertMember(H5std_string("household_ID"), HOFFSET(PersonTIDataType, household_ID), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("school_ID"), HOFFSET(PersonTIDataType, school_ID), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("work_ID"), HOFFSET(PersonTIDataType, work_ID), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("prim_comm_ID"), HOFFSET(PersonTIDataType, prim_comm_ID), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("sec_comm_ID"), HOFFSET(PersonTIDataType, sec_comm_ID), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("start_infectiousness"),
									  HOFFSET(PersonTIDataType, start_infectiousness), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("time_infectiousness"),
									  HOFFSET(PersonTIDataType, time_infectiousness), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("start_symptomatic"),
									  HOFFSET(PersonTIDataType, start_symptomatic), PredType::NATIVE_UINT);
			typePersonTI.insertMember(H5std_string("time_symptomatic"),
									  HOFFSET(PersonTIDataType, time_symptomatic), PredType::NATIVE_UINT);

			DataSpace* dataspace = new DataSpace(1, dims);
			DataSet* dataset = new DataSet(file.createDataSet("personsTI", typePersonTI, *dataspace));

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
					personData[j].ID = sim.getPopulation().get()->at(i).m_id;
					personData[j].age = sim.getPopulation().get()->at(i).m_age;
					personData[j].gender = sim.getPopulation().get()->at(i).m_gender;
					personData[j].household_ID = sim.getPopulation().get()->at(i).m_household_id;
					personData[j].school_ID = sim.getPopulation().get()->at(i).m_school_id;
					personData[j].work_ID = sim.getPopulation().get()->at(i).m_work_id;
					personData[j].prim_comm_ID = sim.getPopulation().get()->at(i).m_primary_community_id;
					personData[j].sec_comm_ID = sim.getPopulation().get()->at(i).m_secondary_community_id;
					personData[j].start_infectiousness = sim.getPopulation().get()->at(i).m_health.getStartInfectiousness();
					personData[j].start_symptomatic = sim.getPopulation().get()->at(i).m_health.getStartSymptomatic();
					personData[j].time_infectiousness =
							sim.getPopulation().get()->at(i).m_health.getEndInfectiousness() - sim.getPopulation().get()->at(i).m_health.getStartInfectiousness();
					personData[j].time_symptomatic =
							sim.getPopulation().get()->at(i).m_health.getEndSymptomatic() - sim.getPopulation().get()->at(i).m_health.getStartSymptomatic();
					i++;
				}

				// Select a hyperslab in the dataset.
				DataSpace *filespace = new DataSpace(dataset->getSpace ());
				hsize_t offset[1] = {i-selected_dims[0]};
				filespace->selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

				// Define memory space.
				DataSpace *memspace = new DataSpace(1, selected_dims, NULL);

				// Write data to the selected portion of the dataset.
				dataset->write(personData, typePersonTI, *memspace, *filespace);
				delete filespace;
				delete memspace;
			}
			delete dataspace;
			delete dataset;
		}



		// Save amt_timesteps
		DataSet timeset = file.openDataSet("amt_timesteps");
		unsigned int amt_timesteps[1] = {m_save_count};
		timeset.write(amt_timesteps, PredType::NATIVE_UINT);

		// Save the last timestep (current one that is being saved)
		DataSet last_timestepset = file.openDataSet("last_timestep");
		unsigned int last_timestep[1] = {m_timestep};
		last_timestepset.write(last_timestep, PredType::NATIVE_UINT);

		std::stringstream ss;
		ss << "/Timestep_" << m_timestep;

		Group group(file.createGroup(ss.str()));


		// Save Random Number Generator
		hsize_t dims[1];
		dims[0] = sim.m_num_threads;

		DataSpace* dataspace = new DataSpace(1, dims);
		CompType typeRng(sizeof(RNGDataType));
		typeRng.insertMember(H5std_string("seed"), HOFFSET(RNGDataType, seed), PredType::NATIVE_ULONG);
		StrType tid1(0, H5T_VARIABLE);
		typeRng.insertMember(H5std_string("state"), HOFFSET(RNGDataType, rng_state), tid1);
		DataSet* dataset = new DataSet(group.createDataSet("randomgen", typeRng, *dataspace));

		RNGDataType rngs[sim.m_num_threads];
		std::vector<std::string> rng_states = sim.getRngStates();
		for (unsigned int i = 0; i < sim.m_rng_handler.size(); i++) {
			rngs[i].seed = sim.m_rng_handler.at(i).getSeed();
			std::string str = rng_states[i];
			rngs[i].rng_state = str.c_str();
		}
		dataset->write(rngs, typeRng);

		delete dataspace;
		delete dataset;


		// Save Calendar
		dims[0] = 1;
		CompType typeCalendar(sizeof(CalendarDataType));
		typeCalendar.insertMember(H5std_string("day"), HOFFSET(CalendarDataType, day), PredType::NATIVE_HSIZE);
		typeCalendar.insertMember(H5std_string("date"), HOFFSET(CalendarDataType, date), tid1);
		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("Calendar", typeCalendar, *dataspace));

		CalendarDataType calendar[1];
		calendar[0].day = sim.m_calendar->getSimulationDay();
		ss.str("");
		ss.clear();
		ss << sim.m_calendar->getYear() << "-" << sim.m_calendar->getMonth() << "-" << sim.m_calendar->getDay();
		calendar[0].date = ss.str().c_str();
		dataset->write(calendar, typeCalendar);

		delete dataspace;
		delete dataset;


		// Save Person Time Dependent
		dims[0] = sim.getPopulation()->size();
		CompType typePersonTD(sizeof(PersonTDDataType));
		typePersonTD.insertMember(H5std_string("at_household"),
								  HOFFSET(PersonTDDataType, at_household), PredType::NATIVE_HBOOL);
		typePersonTD.insertMember(H5std_string("at_school"),
								  HOFFSET(PersonTDDataType, at_school), PredType::NATIVE_HBOOL);
		typePersonTD.insertMember(H5std_string("at_work"),
								  HOFFSET(PersonTDDataType, at_work), PredType::NATIVE_HBOOL);
		typePersonTD.insertMember(H5std_string("at_prim_comm"),
								  HOFFSET(PersonTDDataType, at_prim_comm), PredType::NATIVE_HBOOL);
		typePersonTD.insertMember(H5std_string("at_sec_comm"),
								  HOFFSET(PersonTDDataType, at_sec_comm), PredType::NATIVE_HBOOL);
		typePersonTD.insertMember(H5std_string("participant"),
								  HOFFSET(PersonTDDataType, participant), PredType::NATIVE_HBOOL);
		typePersonTD.insertMember(H5std_string("health_status"),
								  HOFFSET(PersonTDDataType, health_status), PredType::NATIVE_UINT);
		typePersonTD.insertMember(H5std_string("disease_counter"),
								  HOFFSET(PersonTDDataType, disease_counter), PredType::NATIVE_UINT);

		// Dataspace can fit all persons but is chunked in parts of 100 persons
		dataspace = new DataSpace(1, dims);
		DSetCreatPropList  *plist = new  DSetCreatPropList;
		hsize_t chunk_dims[1] = {40000};
		plist->setChunk(1, chunk_dims);
		dataset = new DataSet(group.createDataSet("PersonTD", typePersonTD, *dataspace, *plist));

		// Persons are saved per chunk
		unsigned int i = 0;
		while (i < dims[0]) {
			hsize_t selected_dims[1];
			if (i + chunk_dims[0] < dims[0]) {
				selected_dims[0] = chunk_dims[0];
			} else {
				selected_dims[0] = dims[0] - i;
			}

			PersonTDDataType personData[selected_dims[0]];
			for (unsigned int j = 0; j < selected_dims[0]; j++) {
				personData[j].at_household = sim.getPopulation().get()->at(i).m_at_household;
				personData[j].at_school = sim.getPopulation().get()->at(i).m_at_school;
				personData[j].at_work = sim.getPopulation().get()->at(i).m_at_work;
				personData[j].at_prim_comm = sim.getPopulation().get()->at(i).m_at_primary_community;
				personData[j].at_sec_comm = sim.getPopulation().get()->at(i).m_at_secondary_community;
				personData[j].participant = sim.getPopulation().get()->at(i).m_is_participant;
				personData[j].health_status = (unsigned int) sim.getPopulation().get()->at(i).m_health.getHealthStatus();
				personData[j].disease_counter = (unsigned int) sim.getPopulation().get()->at(i).m_health.getDiseaseCounter();
				i++;
			}

			// Select a hyperslab in the dataset.
			DataSpace *filespace = new DataSpace(dataset->getSpace ());
			hsize_t offset[1] = {i-selected_dims[0]};
			filespace->selectHyperslab(H5S_SELECT_SET, selected_dims, offset);

			// Define memory space.
			DataSpace *memspace = new DataSpace(1, selected_dims, NULL);

			// Write data to the selected portion of the dataset.
			dataset->write(personData, typePersonTD, *memspace, *filespace);
			delete filespace;
			delete memspace;
		}

		dataspace->close();
		dataset->close();
		delete plist;
		m_timestep += m_frequency;
		file.close();
	} catch (GroupIException error) {
		error.printError();
		return;
	} catch (AttributeIException error) {
		error.printError();
		return;
	} catch (FileIException error) {
		std::cout << "Trying to open file: " << m_filename << " but failed." << std::endl;
		error.printError();
		return;
	} catch (DataSetIException error) {
		error.printError();
		return;
	} catch (DataSpaceIException error) {
		error.printError();
		return;
	} catch (Exception error) {
		std::cout << "Unknown exception?" << std::endl;
		error.printError();
	}
	return;
}


}
