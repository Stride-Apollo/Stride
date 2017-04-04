/**
 * @file
 * Source file for the Loader class for the checkpointing functionality
 */

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "Loader.h"
#include "calendar/Calendar.h"
#include "util/InstallDirs.h"
#include "pop/Population.h"
#include "checkpointing/customDataTypes/CalendarDataType.h"
#include "checkpointing/customDataTypes/ConfDataType.h"
#include "checkpointing/customDataTypes/PersonTDDataType.h"
#include "checkpointing/customDataTypes/PersonTIDataType.h"
#include "checkpointing/customDataTypes/RNGDataType.h"
#include "sim/SimulatorBuilder.h"
#include "pop/PopulationBuilder.h"

using namespace H5;
using namespace boost::property_tree;
using namespace std;
using namespace boost::filesystem;
using namespace stride::util;

namespace stride {
Loader::Loader(const char *filename, unsigned int num_threads): m_filename(filename), m_num_threads(num_threads) {
	try {
		m_num_threads = num_threads;
		Exception::dontPrint();
		H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
		std::cout << "Start load\n";
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
		typeConf.insertMember(H5std_string("population_file"), HOFFSET(ConfDataType, population_file), tid1);
		typeConf.insertMember(H5std_string("disease_config_file"), HOFFSET(ConfDataType, disease_config_file), tid1);
		typeConf.insertMember(H5std_string("holidays_file"), HOFFSET(ConfDataType, holidays_file), tid1);
		typeConf.insertMember(H5std_string("age_contact_matrix_file"), HOFFSET(ConfDataType, age_contact_matrix_file), tid1);
		typeConf.insertMember(H5std_string("checkpointing_file"), HOFFSET(ConfDataType, checkpointing_file), tid1);

		ConfDataType configData[1];
		std::cout << "first open\n";
		DataSet* dataset = new DataSet(file.openDataSet("configuration/configuration"));
		std::cout << "first read\n";
		dataset->read(configData, typeConf);

		m_pt_config.add("run.checkpointing_frequency", configData[0].checkpointing_frequency);
		m_pt_config.add("run.rng_seed", configData[0].rng_seed);
		m_pt_config.add("run.r0", configData[0].r0);
		m_pt_config.add("run.seeding_rate", configData[0].seeding_rate);
		m_pt_config.add("run.immunity_rate", configData[0].immunity_rate);
		m_pt_config.add("run.num_days", configData[0].num_days);
		m_pt_config.add("run.output_prefix", std::string(configData[0].output_prefix));
		m_pt_config.add("run.generate_person_file", configData[0].generate_person_file ? 1 : 0);
		m_pt_config.add("run.num_participants_survey", configData[0].num_participants_survey);
		m_pt_config.add("run.start_date", std::string(configData[0].start_date));
		m_pt_config.add("run.log_level", std::string(configData[0].log_level));
		m_pt_config.add("run.population_file", std::string(configData[0].population_file));
		m_pt_config.add("run.disease_config_file", std::string(configData[0].disease_config_file));
		m_pt_config.add("run.holidays_file", std::string(configData[0].holidays_file));
		m_pt_config.add("run.age_contact_matrix_file", std::string(configData[0].age_contact_matrix_file));
		m_pt_config.add("run.checkpointing_file", std::string(configData[0].checkpointing_file));
		std::cout << "parse tree\n";
		delete dataset;

		dataset = new DataSet(file.openDataSet("track_index_case"));
		std::cout << "Second open\n";
		bool track[1];
		dataset->read(track, PredType::NATIVE_HBOOL);
		std::cout << "Second read\n";
		m_track_index_case = track[0];

		delete dataset;
		std::cout << "\nDone with the initial loading!!\n\n";

		dataset = new DataSet(file.openDataSet("personsTI"));
		DataSpace dataspace = dataset->getSpace();
		const int ndims = dataspace.getSimpleExtentNdims();
		hsize_t dims[ndims];
		dataspace.getSimpleExtentDims(dims, NULL);
		dataspace.close();

		const auto file_name_d {m_pt_config.get<string>("run.disease_config_file")};
		const auto file_path_d {InstallDirs::getDataDir() /= file_name_d};
		if (!is_regular_file(file_path_d)) {
			throw runtime_error(std::string(__func__) + "> No file " + file_path_d.string());
		}
		read_xml(file_path_d.string(), m_pt_disease);

		const auto file_name_c {m_pt_config.get<string>("run.age_contact_matrix_file")};
		const auto file_path_c {InstallDirs::getDataDir() /= file_name_c};
		if (!is_regular_file(file_path_c)) {
			throw runtime_error(std::string(__func__) + "> No file " + file_path_c.string());
		}
		read_xml(file_path_c.string(), m_pt_contact);

		const auto seed = m_pt_config.get<double>("run.rng_seed");
		Random rng(seed);

		dataset->close();
		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}

void Loader::setup_population(std::shared_ptr<Simulator> sim) {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	DataSet* dataset = new DataSet(file.openDataSet("personsTI"));
	DataSpace dataspace = dataset->getSpace();
	const int ndims = dataspace.getSimpleExtentNdims();
	hsize_t dims[ndims];
	dataspace.getSimpleExtentDims(dims, NULL);
	dataspace.close();

	ptree pt_disease;
	const auto file_name_d {m_pt_config.get<string>("run.disease_config_file")};
	const auto file_path_d {InstallDirs::getDataDir() /= file_name_d};
	if (!is_regular_file(file_path_d)) {
		throw runtime_error(std::string(__func__) + "> No file " + file_path_d.string());
	}
	read_xml(file_path_d.string(), pt_disease);

	ptree pt_contact;
	const auto file_name_c {m_pt_config.get<string>("run.age_contact_matrix_file")};
	const auto file_path_c {InstallDirs::getDataDir() /= file_name_c};
	if (!is_regular_file(file_path_c)) {
		throw runtime_error(std::string(__func__) + "> No file " + file_path_c.string());
	}
	read_xml(file_path_c.string(), pt_contact);

	const auto seed = m_pt_config.get<double>("run.rng_seed");
	Random rng(seed);

	sim = SimulatorBuilder::build(m_pt_config, pt_disease, pt_contact, m_num_threads, m_track_index_case);

	//sim->m_population = PopulationBuilder::build(m_pt_config, pt_disease, rng);

	/*CompType typePersonTI(sizeof(PersonTIDataType));
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

	for (unsigned int i = 0; i < dims[0]; i++) {
		PersonTIDataType person[1];
		hsize_t dim_sub[1] = {1};

		DataSpace memspace(1, dim_sub, NULL);

		hsize_t offset[1] = {i};
		hsize_t count[1] = {1};
		hsize_t stride[1] = {1};
		hsize_t block[1] = {1};

		dataspace = dataset->getSpace();
		dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
		dataset->read(person, typePersonTI, memspace, dataspace);
		sim->m_population.get()->push_back(Simulator::PersonType(person[0].ID, person[0].age, person[0].household_ID, person[0].school_ID, person[0].work_ID, person[0].prim_comm_ID, person[0].sec_comm_ID, person[0].start_infectiousness, person[0].start_symptomatic, person[0].time_infectiousness, person[0].time_symptomatic));
		memspace.close();
		dataspace.close();
	}*/
	dataset->close();
	file.close();
}

}

