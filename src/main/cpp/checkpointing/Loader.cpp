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
#include "sim/SimulatorBuilder.h"

using namespace H5;
using namespace boost::property_tree;

namespace stride {
std::shared_ptr<Simulator> Loader::build_sim(unsigned int num_threads) {
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

		DataSet* dataset = new DataSet(file.openDataSet("configuration/configuration"));
		dataset->read(configData, typeConf);

		ptree pt_config;
		pt_config.add("run.checkpointing_frequency", configData[0].checkpointing_frequency);
		pt_config.add("run.rng_seed", configData[0].rng_seed);
		pt_config.add("run.r0", configData[0].r0);
		pt_config.add("run.seeding_rate", configData[0].seeding_rate);
		pt_config.add("run.immunity_rate", configData[0].immunity_rate);
		pt_config.add("run.num_days", configData[0].num_days);
		pt_config.add("run.output_prefix", configData[0].output_prefix);
		pt_config.add("run.generate_person_file", configData[0].generate_person_file);
		pt_config.add("run.num_participants_survey", configData[0].num_participants_survey);
		pt_config.add("run.start_date", configData[0].start_date);
		pt_config.add("run.log_level", configData[0].log_level);

		delete dataset;

		dataset = new DataSet(file.openDataSet("configuration/track_index_case"));
		bool track[1];
		dataset->read(track, PredType::NATIVE_HBOOL);
		dataset->close();

		file.close();
		m_track_index_case = track[0];
		return SimulatorBuilder::build(pt_config, num_threads, m_track_index_case);
	} catch(FileIException error) {
		error.printError();
		return nullptr;
	}
}
}

