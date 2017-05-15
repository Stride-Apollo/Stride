#pragma once

#include <string>
#include "sim/SimulatorRunMode.h"

namespace stride {

using namespace std;

void run_stride(bool track_index_case,
				unsigned int num_threads,
				const string& config_file_name,
				const string& hdf5_file_name,
				const string& hdf5_output_file_name,
				const int checkpointing_frequency,
				const unsigned int timestamp_replay,
				RunMode run_mode);

}
