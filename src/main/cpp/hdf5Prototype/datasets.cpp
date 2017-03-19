#include <iostream>
#include <string>

#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
	using namespace H5;
	#ifndef H5_NO_STD
		using std::cout;
		using std::endl;
	#endif
#endif

const H5std_string FILE_NAME("h5tutr_group.h5");
const H5std_string DATASET_NAME("dset");
const H5std_string ATTR_NAME("units");
const int DIM1 = 2;

int main() {
	int attr_data[2] = { 100, 200};
	hsize_t dims[1] = { DIM1 };

	try {
		Exception::dontPrint();
		H5File file(FILE_NAME, H5F_ACC_RDWR);
		DataSet dataset = file.openDataSet(DATASET_NAME);
		DataSpace attr_dataspace = DataSpace(1, dims);
		Attribute attribute = dataset.createAttribute(ATTR_NAME, PredType::STD_I32BE, attr_dataspace);
		attribute.write(PredType::NATIVE_INT, attr_data);
	} catch(AttributeIException error) {
		error.printError();
		return -1;
	} catch(FileIException error) {
		error.printError();
		return -1;
	} catch(DataSetIException error) {
		error.printError();
		return -1;
	} catch(DataSpaceIException error) {
		error.printError();
		return -1;
	}
	return 0;
}
