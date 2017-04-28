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
#include "core/Cluster.h"

#include <vector>
#include <string>

using namespace H5;
using namespace boost::property_tree;
using namespace std;
using namespace boost::filesystem;
using namespace stride::util;

namespace stride {
Loader::Loader(const char* filename): m_filename(filename) {
	try{
		H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
		StrType tid1(0, H5T_VARIABLE);
		CompType typeConfData(sizeof(ConfDataType));
		typeConfData.insertMember(H5std_string("conf_content"), HOFFSET(ConfDataType, conf_content), tid1);
		typeConfData.insertMember(H5std_string("disease_content"), HOFFSET(ConfDataType, disease_content), tid1);
		typeConfData.insertMember(H5std_string("age_contact_content"), HOFFSET(ConfDataType, age_contact_content), tid1);
		typeConfData.insertMember(H5std_string("holidays_content"), HOFFSET(ConfDataType, holidays_content), tid1);
		ConfDataType configData[1];
		DataSet* dataset = new DataSet(file.openDataSet("configuration/configuration"));
		dataset->read(configData, typeConfData);

		istringstream iss(configData[0].conf_content);

		std::ofstream output_stream;
		stringstream output;
		output << filename << "_config.xml";
		output_stream.open(output.str());
		output_stream << iss.rdbuf();

		iss.clear();
		iss.str("");
		iss.str(configData[0].disease_content);
		output.clear();
		output.str("");
		output << filename << "_disease.xml";
		output_stream.close();
		output_stream.clear();
		output_stream.flush();
		output_stream.open(output.str());
		output_stream << iss.rdbuf();

		iss.clear();
		iss.str("");
		iss.str(configData[0].age_contact_content);
		output.clear();
		output.str("");
		output << filename << "_contact.xml";
		output_stream.close();
		output_stream.clear();
		output_stream.flush();
		output_stream.open(output.str());
		output_stream << iss.rdbuf();

		iss.clear();
		iss.str("");
		iss.str(configData[0].holidays_content);
		output.clear();
		output.str("");
		output << filename << "_holidays.json";
		output_stream.close();
		output_stream.clear();
		output_stream.flush();
		output_stream.open(output.str());
		output_stream << iss.rdbuf();

		dataset->close();
		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}

Loader::Loader(const char *filename, unsigned int num_threads): m_filename(filename), m_num_threads(num_threads) {
	try {
		m_num_threads = num_threads;
		Exception::dontPrint();
		H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);


		StrType tid1(0, H5T_VARIABLE);
		CompType typeConfData(sizeof(ConfDataType));
		typeConfData.insertMember(H5std_string("conf_content"), HOFFSET(ConfDataType, conf_content), tid1);
		typeConfData.insertMember(H5std_string("disease_content"), HOFFSET(ConfDataType, disease_content), tid1);
		typeConfData.insertMember(H5std_string("age_contact_content"), HOFFSET(ConfDataType, age_contact_content), tid1);
		typeConfData.insertMember(H5std_string("holidays_content"), HOFFSET(ConfDataType, holidays_content), tid1);
		ConfDataType configData[1];
		DataSet* dataset = new DataSet(file.openDataSet("configuration/configuration"));
		dataset->read(configData, typeConfData);

		istringstream iss(configData[0].conf_content);
		xml_parser::read_xml(iss, m_pt_config);
		iss.clear();
		iss.str("");
		iss.str(configData[0].disease_content);
		xml_parser::read_xml(iss, m_pt_disease);
		iss.clear();
		iss.str("");
		iss.str(configData[0].age_contact_content);
		xml_parser::read_xml(iss, m_pt_contact);

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

		// const auto seed = m_pt_config.get<double>("run.rng_seed");
		// Random rng(seed);

		dataset->close();
		file.close();
	} catch(FileIException error) {
		error.printError();
	}
}

void Loader::setupPopulation(std::shared_ptr<Simulator> sim) {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	DataSet* dataset = new DataSet(file.openDataSet("personsTI"));
	DataSpace dataspace = dataset->getSpace();
	const int ndims = dataspace.getSimpleExtentNdims();
	hsize_t dims[ndims];
	dataspace.getSimpleExtentDims(dims, NULL);
	dataspace.close();

	const auto seed = m_pt_config.get<double>("run.rng_seed");
	Random rng(seed);

	sim = SimulatorBuilder::build(m_pt_config, m_pt_disease, m_pt_contact, m_num_threads, m_track_index_case);

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

void Loader::extendSimulation(std::shared_ptr<Simulator> sim) {
	loadFromTimestep(this->getLastSavedTimestep(), sim);
}

void Loader::loadFromTimestep(unsigned int timestep, std::shared_ptr<Simulator> sim) {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	CompType typeCalendar(sizeof(CalendarDataType));
	StrType tid1(0, H5T_VARIABLE);
	typeCalendar.insertMember(H5std_string("day"), HOFFSET(CalendarDataType, day), PredType::NATIVE_HSIZE);
	typeCalendar.insertMember(H5std_string("date"), HOFFSET(CalendarDataType, date), tid1);
	std::stringstream ss;
	ss << "/Timestep_" << timestep;
	DataSet* dataset = new DataSet(file.openDataSet(ss.str() + "/Calendar"));
	CalendarDataType calendar[1];
	dataset->read(calendar, typeCalendar);

	sim->m_calendar = std::make_shared<Calendar>(m_pt_config);
	sim->m_calendar.get()->m_day = calendar[0].day;
	sim->m_calendar.get()->m_date = boost::gregorian::from_simple_string(calendar[0].date);

	delete dataset;


	// Set up rng states
	dataset = new DataSet(file.openDataSet(ss.str() + "/randomgen"));

	DataSpace dataspace_rng = dataset->getSpace();
	const int ndims_rng = dataspace_rng.getSimpleExtentNdims();
	hsize_t dims_rng[ndims_rng];
	dataspace_rng.getSimpleExtentDims(dims_rng, NULL);
	dataspace_rng.close();

	CompType typeRng(sizeof(RNGDataType));
	typeRng.insertMember(H5std_string("seed"), HOFFSET(RNGDataType, seed), PredType::NATIVE_ULONG);
	typeRng.insertMember(H5std_string("state"), HOFFSET(RNGDataType, rng_state), tid1);

	vector<string> states;
	for (unsigned int i = 0; i < dims_rng[0]; i++) {
		RNGDataType rng_state[1];
		hsize_t dim_sub[1] = {1};
		DataSpace memspace(1, dim_sub, NULL);

		hsize_t offset[1] = {i};
		hsize_t count[1] = {1};
		hsize_t stride[1] = {1};
		hsize_t block[1] = {1};

		DataSpace dataspace = dataset->getSpace();
		dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
		dataset->read(rng_state, typeRng, memspace, dataspace);

		string state = rng_state[0].rng_state;
		states.push_back(state);

		memspace.close();
		dataspace.close();
	}
	sim->setRngStates(states);
	delete dataset;


	dataset = new DataSet(file.openDataSet(ss.str() + "/PersonTD"));
	unsigned long dims[1] = {sim->m_population->size()};
	CompType typePersonTD(sizeof(PersonTDDataType));
	typePersonTD.insertMember(H5std_string("at_household"),
							  HOFFSET(PersonTDDataType, at_household), PredType::NATIVE_INT);
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
	for (unsigned int i = 0; i < dims[0]; i++) {
		PersonTDDataType person[1];
		hsize_t dim_sub[1] = {1};

		DataSpace memspace(1, dim_sub, NULL);

		hsize_t offset[1] = {i};
		hsize_t count[1] = {1};
		hsize_t stride[1] = {1};
		hsize_t block[1] = {1};

		DataSpace dataspace = dataset->getSpace();
		dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
		dataset->read(person, typePersonTD, memspace, dataspace);
		sim.get()->m_population.get()->at(i).m_at_household = person[0].at_household;
		sim.get()->m_population.get()->at(i).m_at_work = person[0].at_work;
		sim.get()->m_population.get()->at(i).m_at_school = person[0].at_school;
		sim.get()->m_population.get()->at(i).m_at_primary_community = person[0].at_prim_comm;
		sim.get()->m_population.get()->at(i).m_at_secondary_community = person[0].at_sec_comm;
		sim.get()->m_population.get()->at(i).m_is_participant = person[0].participant;
		sim.get()->m_population.get()->at(i).m_health.m_status = HealthStatus(person[0].health_status);
		sim.get()->m_population.get()->at(i).m_health.m_disease_counter = person[0].disease_counter;
		memspace.close();
		dataspace.close();
	}
	this->updateClusterImmuneIndices(sim);

	dataset->close();
	file.close();
}

int Loader::getLastSavedTimestep() const {
	H5File file(m_filename, H5F_ACC_RDONLY, H5P_DEFAULT, H5P_DEFAULT);
	DataSet* dataset = new DataSet(file.openDataSet("amt_timesteps"));
	unsigned int data[1];
	dataset->read(data, PredType::NATIVE_UINT);
	dataset->close();
	file.close();
	return data[0];
}

void Loader::updateClusterImmuneIndices(std::shared_ptr<Simulator> sim) const {
	for (auto cluster : sim->m_households) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_school_clusters) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_work_clusters) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_primary_community) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
	for (auto cluster : sim->m_secondary_community) {
		cluster.m_index_immune = cluster.m_members.size()-1;
	}
}


}

