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
		m_track_index_case = track[0];

		delete dataset;

		std::shared_ptr<Simulator> sim = SimulatorBuilder::build(pt_config, num_threads, m_track_index_case);

		dataset = new DataSet(file.openDataSet("personsTI"));
		DataSpace dataspace = dataset->getSpace();
		const int ndims = dataspace.getSimpleExtentNdims();
		hsize_t dims[ndims];
		dataspace.getSimpleExtentDims(dims, NULL);
		dataspace.close();

		sim->m_population = std::make_shared<Population>();

		CompType typePersonTI(sizeof(PersonTIDataType));
		typePersonTI.insertMember(H5std_string("ID"), HOFFSET(PersonTIDataType, ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("age"), HOFFSET(PersonTIDataType, age), PredType::NATIVE_DOUBLE);
		typePersonTI.insertMember(H5std_string("gender"), HOFFSET(PersonTIDataType, gender), PredType::NATIVE_CHAR);
		typePersonTI.insertMember(H5std_string("household_ID"), HOFFSET(PersonTIDataType, household_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("school_ID"), HOFFSET(PersonTIDataType, school_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("work_ID"), HOFFSET(PersonTIDataType, work_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("prim_comm_ID"), HOFFSET(PersonTIDataType, prim_comm_ID), PredType::NATIVE_UINT);
		typePersonTI.insertMember(H5std_string("sec_comm_ID"), HOFFSET(PersonTIDataType, sec_comm_ID), PredType::NATIVE_UINT);

		for (unsigned int i = 0; i < dims[0]; i++) {
			PersonTIDataType person[1];
			hsize_t dim_sub[1] = {100};

			DataSpace memspace(1, dim_sub, NULL);

			hsize_t offset[1] = {i};
			hsize_t count[1] = {1};
			hsize_t stride[1] = {1};
			hsize_t block[1] = {1};

			dataspace = dataset->getSpace();
			dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
			dataset->read(person, typePersonTI, dataspace);
			sim->m_population.get()->push_back(Simulator::PersonType(person[0].ID, person[0].age, person[0].household_ID, person[0].school_ID, person[0].work_ID, person[0].prim_comm_ID, person[0].sec_comm_ID, person[0].start_infectiousness, person[0].start_symptomatic, person[0].time_infectiousness, person[0].time_symptomatic));
		}

		dataset->close();
		file.close();
		std::cout << "\nDone with the initial loading!!\n\n";

		return sim;
	} catch(FileIException error) {
		error.printError();
		return nullptr;
	}
}
}

