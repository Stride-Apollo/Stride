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

		StrType tid1(0, H5T_VARIABLE);
		ConfDataType configData[1];
		DataSet* dataset = new DataSet(file.openDataSet("configuration/configuration"));
		dataset->read(configData, tid1);

		istringstream iss(configData[0].conf_content);
		xml_parser::read_xml(iss, m_pt_config);
		delete dataset;

		dataset = new DataSet(file.openDataSet("track_index_case"));
		bool track[1];
		dataset->read(track, PredType::NATIVE_HBOOL);
		m_track_index_case = track[0];

		delete dataset;

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

void Loader::load_from_timestep(unsigned int timestep, std::shared_ptr<Simulator> sim) {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	CompType typeCalendar(sizeof(CalendarDataType));
	StrType tid1(0, H5T_VARIABLE);
	typeCalendar.insertMember(H5std_string("day"), HOFFSET(CalendarDataType, day), PredType::NATIVE_HSIZE);
	typeCalendar.insertMember(H5std_string("date"), HOFFSET(CalendarDataType, date), tid1);
	std::stringstream ss;
	ss << "/Timestep_" << timestep;
	std::cout << "Loading: " << ss.str() << "\n";
	DataSet dataset = file.openDataSet(ss.str() + "/Calendar");
	CalendarDataType calendar[1];
	dataset.read(calendar, typeCalendar);

	sim->m_calendar = std::make_shared<Calendar>(m_pt_config);
	sim->m_calendar.get()->m_day = calendar[0].day;
	sim->m_calendar.get()->m_date = boost::gregorian::from_simple_string(calendar[0].date);

	std::cout << "Initialized calendar to " << calendar[0].day << " " << calendar[0].date << "\n";

	dataset.close();
	file.close();
}

}

