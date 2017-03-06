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

const H5std_string FILE_NAME("h5tutr_groups.h5");
const H5std_string DATASET_NAME1("/MyGroup/dset1");
const H5std_string DATASET_NAME2("dset2");
const int RANK = 2;
const int D1DIM1 = 3;
const int D1DIM2 = 3;
const int D2DIM1 = 2;
const int D2DIM2 = 10;

int main() {
	int dset1_data[D1DIM1][D1DIM2], dset2_data[D2DIM1][D2DIM2];
	int i,j;

	try {
		Exception::dontPrint();
		H5File file(FILE_NAME, H5F_ACC_RDWR);
		for (i=0; i < D1DIM1; i++) {
			for (j=0; j< D1DIM2; j++) {
				dset1_data[i][j] = j+1;
			}
		}
		for (i=0; i < D2DIM1; i++) {
			for (j=0; j< D2DIM2; j++) {
				dset2_data[i][j] = j+1;
			}
		}

		hsize_t dims[RANK];
		dims[0] = D1DIM1;
		dims[1] = D1DIM2;
		DataSpace* dataspace = new DataSpace(RANK, dims);
		DataSet* dataset = new DataSet(file.createDataSet(DATASET_NAME1, PredType::STD_I32BE, *dataspace));
		dataset->write(dset1_data, PredType::NATIVE_INT);

		delete dataset;
		delete dataspace;

		dims[0] = D2DIM1;
		dims[1] = D2DIM2;
		dataspace = new DataSpace(RANK, dims);
		Group group(file.openGroup("/MyGroup/Group_A"));
		dataset = new DataSet(group.createDataSet(DATASET_NAME2, PredType::STD_I32BE, *dataspace));
		dataset->write(dset2_data, PredType::NATIVE_INT);

		delete dataset;
		delete dataspace;
		group.close();
	} catch(GroupIException error) {
		error.printError();
		return -1;
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
