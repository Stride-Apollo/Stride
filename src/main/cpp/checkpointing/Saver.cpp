/**
 * @file
 * Source file for the Saver class for the checkpointing functionality
 */

#include "Saver.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/RNGDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/CalendarDataType.h"

using namespace H5;

namespace stride {

Saver::Saver(const char* filename): m_filename(filename) {
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
		dimsf[0] = 11;
		DataSpace* dataspace = new DataSpace(1, dimsf);

		CompType typeConf(sizeof(ConfDataType));
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
		typeConf.insertMember(H5std_string("log_level"), HOFFSET(ConfDataType, log_level), PredType::NATIVE_B8);

		DataSet* dataset = new DataSet(group.createDataSet(H5std_string("configuration"), typeConf, *dataspace));

		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("holiday", PredType::NATIVE_INT, *dataspace));
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
		// TODO amt_timesteps
		dataset = new DataSet(file.createDataSet("amt_timesteps", PredType::NATIVE_UINT, *dataspace));


		dataset->close();
		dataspace->close();

		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}

void Saver::update(const Simulator& sim) {
	static int timestep = 0;
	// TODO Implement config stuff in here (x timesteps, filenames, ...)
	try {
		H5File file(m_filename, H5F_ACC_RDWR);

		std::stringstream ss;
		ss << "/Timestep_" << timestep;

		Group group(file.createGroup(ss.str()));

		hsize_t dims[1];
		dims[0] = 1;
		// TODO amt_threads
		DataSpace* dataspace = new DataSpace(1, dims);
		CompType typeRng(sizeof(RNGDataType));
		typeRng.insertMember(H5std_string("seed"), HOFFSET(RNGDataType, seed), PredType::NATIVE_ULONG);
		DataSet* dataset = new DataSet(group.createDataSet("randomgen", typeRng, *dataspace));
		delete dataspace;
		delete dataset;

		CompType typeCalendar(sizeof(CalendarDataType));
		StrType tid1(0, H5T_VARIABLE);
		typeCalendar.insertMember(H5std_string("day"), HOFFSET(CalendarDataType, day), PredType::NATIVE_HSIZE);
		typeCalendar.insertMember(H5std_string("date"), HOFFSET(CalendarDataType, date), tid1);
		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("Calendar", typeCalendar, *dataspace));

		delete dataspace;
		delete dataset;

		// TODO amt_persons
		// TODO For some reason the typePersonTD Compound data type throws an exception :/
		/*CompType typePersonTD(sizeof(PersonTDDataType));
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

		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("PersonTD", typePersonTD, *dataspace));

		dataspace->close();
		dataset->close();*/
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
