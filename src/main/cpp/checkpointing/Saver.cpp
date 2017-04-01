/**
 * @file
 * Source file for the Saver class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include "Saver.h"
#include "calendar/Calendar.h"
#include "checkpointing/customDataTypes/CalendarDataType.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
#include "checkpointing/customDataTypes/RNGDataType.h"

using namespace H5;

namespace stride {

Saver::Saver(const char* filename, ptree pt_config, int frequency): m_filename(filename), m_frequency(frequency), m_pt_config(pt_config) {
	try {
		Exception::dontPrint();
		H5File file(m_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		hsize_t dims[1];
		dims[0] = 1;
		Group group(file.createGroup("/configuration"));

		// TODO Perhaps the dataset of disease is not necessary
		/*DataSpace* dataspace = new DataSpace(1, dims);
		DataSet* dataset = new DataSet(group.createDataSet("disease", PredType::STD_I32BE, *dataspace));
		delete dataspace;
		delete dataset;*/

		hsize_t dimsf[1];
		dimsf[0] = 1;
		DataSpace* dataspace = new DataSpace(1, dimsf);

		typeConf = CompType(sizeof(ConfDataType));
		typeConf.insertMember(H5std_string("checkpointing_frequency"),
							  HOFFSET(ConfDataType, checkpointing_frequency), PredType::NATIVE_UINT);
		typeConf.insertMember(H5std_string("rng_seed"), HOFFSET(ConfDataType, rng_seed), PredType::NATIVE_ULONG);
		typeConf.insertMember(H5std_string("r0"), HOFFSET(ConfDataType, r0), PredType::NATIVE_UINT);
		typeConf.insertMember(H5std_string("seeding_rate"),
							  HOFFSET(ConfDataType, seeding_rate), PredType::NATIVE_DOUBLE);
		typeConf.insertMember(H5std_string("immunity_rate"),
							  HOFFSET(ConfDataType, immunity_rate), PredType::NATIVE_DOUBLE);
		typeConf.insertMember(H5std_string("num_days"), HOFFSET(ConfDataType, num_days), PredType::NATIVE_UINT);
		StrType tid1(0, H5T_VARIABLE);
		typeConf.insertMember(H5std_string("output_prefix"), HOFFSET(ConfDataType, output_prefix), tid1);
		typeConf.insertMember(H5std_string("generate_person_file"),
							  HOFFSET(ConfDataType, generate_person_file), PredType::NATIVE_HBOOL);
		typeConf.insertMember(H5std_string("num_participants_survey"),
							  HOFFSET(ConfDataType, num_participants_survey), PredType::NATIVE_UINT);
		typeConf.insertMember(H5std_string("start_date"), HOFFSET(ConfDataType, start_date), tid1);
		typeConf.insertMember(H5std_string("log_level"), HOFFSET(ConfDataType, log_level), tid1);

		ConfDataType configData[1];
		configData[0].checkpointing_frequency = frequency;
		configData[0].rng_seed = pt_config.get<unsigned long>("run.rng_seed");
		configData[0].r0 = pt_config.get<unsigned int>("run.r0");
		configData[0].seeding_rate = pt_config.get<double>("run.seeding_rate");
		configData[0].immunity_rate = pt_config.get<double>("run.immunity_rate");
		configData[0].num_days = pt_config.get<unsigned int>("run.num_days");
		configData[0].output_prefix = pt_config.get<std::string>("run.output_prefix").c_str();
		int generate = pt_config.get<unsigned int>("run.generate_person_file");
		configData[0].generate_person_file = generate == 1 ? true : false;
		configData[0].num_participants_survey = pt_config.get<unsigned int>("run.num_participants_survey");
		configData[0].start_date = pt_config.get<std::string>("run.start_date").c_str();
		configData[0].log_level = pt_config.get<std::string>("run.log_level").c_str();

		DataSet* dataset = new DataSet(group.createDataSet(H5std_string("configuration"), typeConf, *dataspace));
		dataset->write(configData, typeConf);

		delete dataspace;
		delete dataset;

		/*dataspace = new DataSpace(1, dims);
		// TODO Custom holiday? Still necessary?
		dataset = new DataSet(group.createDataSet("holiday", PredType::NATIVE_INT, *dataspace));
		delete dataspace;
		delete dataset;*/

		hsize_t dims2[2];
		// TODO amt_ages
		dims2[0] = 5;
		dims2[1] = 5;
		dataspace = new DataSpace(2, dims2);
		dataset = new DataSet(group.createDataSet("agecontact", PredType::NATIVE_DOUBLE, *dataspace));

		delete dataspace;
		delete dataset;
		dataspace = new DataSpace(1, dims);

		// TODO amt_persons
		CompType typePersonTI(sizeof(PersonTIDataType));
		typePersonTI.insertMember(H5std_string("ID"), HOFFSET(PersonTIDataType, ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("age"), HOFFSET(PersonTIDataType, age), PredType::NATIVE_DOUBLE);
		typePersonTI.insertMember(H5std_string("gender"), HOFFSET(PersonTIDataType, gender), PredType::NATIVE_CHAR);
		typePersonTI.insertMember(H5std_string("household_ID"), HOFFSET(PersonTIDataType, household_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("school_ID"), HOFFSET(PersonTIDataType, school_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("work_ID"), HOFFSET(PersonTIDataType, work_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("prim_comm_ID"), HOFFSET(PersonTIDataType, prim_comm_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("sec_comm_ID"), HOFFSET(PersonTIDataType, sec_comm_ID), PredType::NATIVE_UINT);

		dataset = new DataSet(file.createDataSet("personsTI", typePersonTI, *dataspace));

		delete dataspace;
		delete dataset;
		dataspace = new DataSpace(1, dims);
		unsigned int amt_time[1] = {0};
		dataset = new DataSet(file.createDataSet("amt_timesteps", PredType::NATIVE_UINT, *dataspace));
		dataset->write(amt_time, PredType::NATIVE_UINT);

		dataset->close();
		dataspace->close();

		file.close();

		testLoad();
	} catch(FileIException error) {
		error.printError();
	}
}

void Saver::testLoad() {
	try {
		H5File file(m_filename, H5F_ACC_RDONLY);
		DataSet dataset = file.openDataSet("configuration/configuration");

		ConfDataType config[1];
		dataset.read(config, typeConf);

		std::cout << "Loaded config file with contents:\n\n";
		std::cout << "Checkpointing frequency: " << config->checkpointing_frequency << std::endl;
		std::cout << "Rng seed: " << config->rng_seed << std::endl;
		std::cout << "R0: " << config->r0 << std::endl;
		std::cout << "Seeding Rate: " << config->seeding_rate << std::endl;
		std::cout << "Immunity Rate: " << config->immunity_rate << std::endl;
		std::cout << "Num Days: " << config->num_days << std::endl;
		std::cout << "Output Prefix: " << config->output_prefix << std::endl;
		std::cout << "Generate Person File: " << config->generate_person_file << std::endl;
		std::cout << "Num Participants Survey: " << config->num_participants_survey << std::endl;
		std::cout << "Start Date: " << config->start_date << std::endl;
		std::cout << "Log Level: " << config->log_level << std::endl;

	} catch (Exception e) {
		e.printError();
	}
}

void Saver::update(const Simulator& sim) {
	static unsigned int timestep = 0;
	static int current_step = 0;
	if (m_frequency != 0 && current_step%m_frequency == 0) {
		try {
			H5File file(m_filename, H5F_ACC_RDWR);

			// Update amt_timesteps
			DataSet timeset = file.openDataSet("amt_timesteps");
			unsigned int amt_timesteps[1] = {timestep};
			timeset.write(amt_timesteps, PredType::NATIVE_UINT);

			std::stringstream ss;
			ss << "/Timestep_" << timestep;

			Group group(file.createGroup(ss.str()));

			hsize_t dims[1];
			dims[0] = sim.m_num_threads;

			DataSpace* dataspace = new DataSpace(1, dims);
			CompType typeRng(sizeof(RNGDataType));
			typeRng.insertMember(H5std_string("seed"), HOFFSET(RNGDataType, seed), PredType::NATIVE_ULONG);
			DataSet* dataset = new DataSet(group.createDataSet("randomgen", typeRng, *dataspace));

			RNGDataType seeds[sim.m_num_threads];
			for (unsigned int i = 0; i < sim.m_rng_handler.size(); i++) {
				seeds[i].seed = sim.m_rng_handler.at(i).getSeed();
			}
			dataset->write(seeds, typeRng);

			delete dataspace;
			delete dataset;

			dims[0] = 1;
			CompType typeCalendar(sizeof(CalendarDataType));
			StrType tid1(0, H5T_VARIABLE);
			typeCalendar.insertMember(H5std_string("day"), HOFFSET(CalendarDataType, day), PredType::NATIVE_HSIZE);
			typeCalendar.insertMember(H5std_string("date"), HOFFSET(CalendarDataType, date), tid1);
			dataspace = new DataSpace(1, dims);
			dataset = new DataSet(group.createDataSet("Calendar", typeCalendar, *dataspace));

			CalendarDataType calendar[1];
			// TODO I suppose this is simulationDay?
			// TODO Fix the super strange seg fault!
			/*std::cout << "Pre Day";
			calendar[0].day = sim.m_calendar->getSimulationDay();
			std::cout << "Day: " << calendar[0].day;
			/*ss.clear();
			ss << sim.m_calendar->getYear() << "-" << sim.m_calendar->getMonth() << "-" << sim.m_calendar->getDay();
			calendar[0].date = ss.str().c_str();
			dataset->write(calendar, typeCalendar);
			std::cout << "Written!";*/

			delete dataspace;
			delete dataset;

			// TODO amt_persons
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

			// Dummy
			/*PersonTDDataType personData[1];
			personData[0].at_household =  true;
			personData[0].at_school =  false;
			*/
			dataspace = new DataSpace(1, dims);
			dataset = new DataSet(group.createDataSet("PersonTD", typePersonTD, *dataspace));

			//dataset->write(personData, typePersonTD);

			dataspace->close();
			dataset->close();
			timestep++;
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
}
