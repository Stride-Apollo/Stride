/**
 * @file
 * Source file for the Loader class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include "Loader.h"
#include "calendar/Calendar.h"
#include "pop/Population.h"
#include "checkpointing/customDataTypes/CalendarDataType.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
#include "checkpointing/customDataTypes/RNGDataType.h"

using namespace H5;

namespace stride {
Loader::Loader(const char* filename, Simulator& sim) : m_filename(filename) {
	try {
		Exception::dontPrint();
		H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);

		CompType typeConf = CompType(sizeof(ConfDataType));
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

		DataSet dataset = file.openDataSet("configuration/configuration");
		dataset.read(configData, typeConf);
		dataset.close();

		

		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}
}

