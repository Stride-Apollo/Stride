#include <iostream>
#include <H5Cpp.h>
#include <string>
#include "Person.h"

using namespace std;
using namespace H5;

/**
 * Reads the hdf5 data file from Createhdf5file.cpp. Prints the content of the data.
 */
int main(int argc, char* argv[]) {
	if (argc != 2) {
		cerr << "Program should be run with 1 argument: the file containing the hdf5 data." << endl;
		return 1;
	}

	string filename = argv[1];
	const H5std_string FILE_NAME(filename);
	const H5std_string DATASET_NAME("PersonsArray");


	try {
		H5File file(FILE_NAME, H5F_ACC_RDONLY);
		DataSet dataset = file.openDataSet(DATASET_NAME);


		CompType typePerson(sizeof(PersonData));
		typePerson.insertMember(H5std_string("age"), HOFFSET(PersonData, age), PredType::NATIVE_INT);
		typePerson.insertMember(H5std_string("height"), HOFFSET(PersonData, height), PredType::NATIVE_INT);
		typePerson.insertMember(H5std_string("gender"), HOFFSET(PersonData, gender), PredType::NATIVE_CHAR);

		hsize_t dims_out[1];
		cout << "Should be 1: " << dataset.getSpace().getSimpleExtentDims(dims_out, NULL) << endl;
		cout << "Should be 3: " << dims_out[0] << endl;

		PersonData persons[3];
		dataset.read(persons, typePerson);

		for (int i = 0; i < 3; i++) {
			cout << "Person " << i << ": age=" << persons[i].age << ", height=" << persons[i].height << ", gender=" << persons[i].gender << endl;
		}

	} catch (Exception e) {
		e.printError();
		return 1;
	}
	return 0;
}
