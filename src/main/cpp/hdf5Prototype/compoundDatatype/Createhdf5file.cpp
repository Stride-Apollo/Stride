#include <iostream>
#include <H5Cpp.h>
#include <string>
#include "Person.h"

using namespace H5;
using namespace std;

/**
 * Create data file with compound datatype.
 */
int main(int argc, char* argv[]) {
	if (argc != 2) {
		cerr << "Program should be run with 1 argument: the filename which should hold the data." << endl;
		return 1;
	}

	char genders[3] = {'M', 'F', 'M'};
	Person persons[3];
	for (int i = 0; i < 3; i++) {
		persons[i] = Person(25+i, 165 + (i*5), genders[i]);
	}
	PersonData data[3];
	for (int i = 0; i < 3; i++) {
		data[i].age = persons[i].getAge();
		data[i].height = persons[i].getHeight();
		data[i].gender = persons[i].getGender();
	}

	try {
		H5File file(argv[1], H5F_ACC_TRUNC);
		
		hsize_t dimsf[1];
		dimsf[0] = 3;
		DataSpace dataspace (1, dimsf);

		CompType typePerson(sizeof(PersonData));
		typePerson.insertMember(H5std_string("age"), HOFFSET(PersonData, age), PredType::NATIVE_INT);
		typePerson.insertMember(H5std_string("height"), HOFFSET(PersonData, height), PredType::NATIVE_INT);
		typePerson.insertMember(H5std_string("gender"), HOFFSET(PersonData, gender), PredType::NATIVE_CHAR);

		DataSet* dataset = new DataSet(file.createDataSet(H5std_string("PersonsArray"), typePerson, dataspace));
		dataset->write(data, typePerson);

		delete dataset;
	} catch (Exception e) {
		e.printError();
		return 1;
	}
	return 0;
}