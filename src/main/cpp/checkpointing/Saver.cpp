/**
 * @file
 * Source file for the Saver class for the checkpointing functionality
 */

#include "Saver.h"
#include <sstream>

using namespace H5;

namespace stride {

Saver::Saver(const char* filename): m_filename(filename) {
	try {
		Exception::dontPrint();
		hid_t file_id;   /* file identifier */
		H5File file(m_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		hsize_t dims[1];
		dims[0] = 1;
		DataSpace* dataspace = new DataSpace(1, dims);
		// TODO Correct custom data types
		Group group(file.createGroup("/configuration"));

		DataSet* dataset = new DataSet(group.createDataSet("disease", PredType::STD_I32BE, *dataspace));

		delete dataspace;
		delete dataset;
		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("holiday", PredType::STD_I32BE, *dataspace));

		delete dataspace;
		delete dataset;
		hsize_t dims2[2];
		// TODO amt_ages
		dims2[0] = 5;
		dims2[1] = 5;
		dataspace = new DataSpace(2, dims2);
		dataset = new DataSet(group.createDataSet("agecontact", PredType::IEEE_F64BE, *dataspace));

		delete dataspace;
		delete dataset;
		dataspace = new DataSpace(1, dims);

		// TODO amt_persons and personTIDataType
		dataset = new DataSet(file.createDataSet("personsTI", PredType::STD_I32BE, *dataspace));

		delete dataspace;
		delete dataset;
		dataspace = new DataSpace(1, dims);
		// TODO amt_timesteps
		dataset = new DataSet(file.createDataSet("amt_timesteps", PredType::STD_U32BE, *dataspace));


		dataset->close();
		dataspace->close();

		H5Fclose(file_id);
	} catch(FileIException error) {
		error.printError();
	}
}

void Saver::update(const Simulator& sim) {
	static int timestep = 0;
	// TODO Implement config stuff in here (x timesteps, filenames, ...)
	try {
		H5File file(m_filename, H5F_ACC_RDWR);

		std::stringstream ss;
		ss << "/Timestep_" << timestep;

		Group group(file.createGroup(ss.str()));

		hsize_t dims[1];
		dims[0] = 1;
		// TODO Correct custom data types
		DataSpace* dataspace = new DataSpace(1, dims);
		// TODO amt_threads
		DataSet* dataset = new DataSet(group.createDataSet("randomgen", PredType::STD_I32BE, *dataspace));
		delete dataspace;
		delete dataset;

		// TODO amt_persons
		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("PersonTD", PredType::STD_I32BE, *dataspace));

		delete dataspace;
		delete dataset;
		dataspace = new DataSpace(1, dims);
		dataset = new DataSet(group.createDataSet("Calendar", PredType::STD_I32BE, *dataspace));

		timestep++;
		dataspace->close();
		dataset->close();
		file.close();
	} catch (GroupIException error) {
		error.printError();
		return;
	} catch (AttributeIException error) {
		error.printError();
		return;
	} catch (FileIException error) {
		std::cout << "Trying to open file: " << m_filename << " but failed." << std::endl;
		error.printError();
		return;
	} catch (DataSetIException error) {
		error.printError();
		return;
	} catch (DataSpaceIException error) {
		error.printError();
		return;
	}
	return;
}
}

/*const H5std_string FILE_NAME("h5tutr_subset.h5");
const H5std_string DATASET_NAME("IntArray");
const int RANK = 2;
const int DIM0_SUB = 3;
const int DIM1_SUB = 4;
const int DIM0 = 8;
const int DIM1 = 10;

int justaFunction() {
	int data[DIM0][DIM1], sdata[DIM0_SUB][DIM1_SUB], rdata[DIM0][DIM1];
	int i,j;

	try {
		Exception::dontPrint();
		H5File file(FILE_NAME, H5F_ACC_TRUNC);

		hsize_t dims[2];
		dims[0] = DIM0;
		dims[1] = DIM1;
		DataSpace dataspace = DataSpace(RANK, dims);
		DataSet dataset(file.createDataSet(DATASET_NAME, PredType::STD_I32BE, dataspace));

		for(j=0; j<DIM0; j++) {
			for(i=0; i<DIM1; i++) {
				if(i < (DIM1/2)) {
					data[j][i] = 1;
				} else {
					data[j][i] = 2;
				}
			}
		}

		dataset.write(data, PredType::NATIVE_INT);

		cout << endl << "Data Written to file:" << endl;
		for (j=0; j<DIM0; j++) {
			for (i=0; i<DIM1; i++) {
				cout << " " << data[j][i];
			}
			cout << endl;
		}
		dataspace.close();
		dataset.close();
		file.close();

		hsize_t offset[2], count[2], stride[2], block[2];
		hsize_t dimsm[2];

		file.openFile(FILE_NAME, H5F_ACC_RDWR);
		dataset = file.openDataSet(DATASET_NAME);

		offset[0] = 1;
		offset[1] = 2;

		count[0] = DIM0_SUB;
		count[1] = DIM1_SUB;

		stride[0] = 1;
		stride[1] = 1;

		block[0] = 1;
		block[1] = 1;

		dimsm[0] = DIM0_SUB;
		dimsm[1] = DIM1_SUB;

		DataSpace memspace(RANK, dimsm, NULL);
		dataspace = dataset.getSpace();
		dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);

		cout << endl << "Write subset to file specifying: " << endl;
		cout << " offset=1x2 stride=1x1 count=3x4 block=1x1" << endl;
		for (j=0; j < DIM0_SUB; j++) {
			for (i=0; i < DIM1_SUB; i++) {
				sdata[j][i] = 5;
			}
		}

		dataset.write(sdata, PredType::NATIVE_INT, memspace, dataspace);
		dataset.read(rdata, PredType::NATIVE_INT);

		cout << endl << "Data in File after Subset is Written:" << endl;
		for (i = 0; i < DIM0; i++) {
			for (j = 0; j < DIM1; j++) {
				cout << " " << rdata[i][j];
			}
			cout << endl;
		}
		cout << endl;

		dataspace.close();
		memspace.close();
		dataset.close();
		file.close();

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
*/