/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of the SummaryFile class.
 */

#include "SummaryFile.h"

#include <omp.h>


namespace stride {
namespace output {

using namespace std;

SummaryFile::SummaryFile(const std::string& file) {
	m_fstream.open((file + "_summary.csv").c_str());

	// add header
	m_fstream << "pop_file,num_days,pop_size,seeding_rate,"
			  << "R0,transm_rate,immunity_rate,num_threads,rng_seed,run_time,"
			  << "total_time,num_cases,AR" << endl;
}

SummaryFile::~SummaryFile() {
	m_fstream.close();
}

void SummaryFile::print(
		const boost::property_tree::ptree& pt_config,
		unsigned int population_size,
		unsigned int num_cases,
		unsigned int run_time,
		unsigned int total_time) {
	unsigned int num_threads = 0;

	#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}

	m_fstream
			<< pt_config.get<string>("run.population_file") << ","
			<< pt_config.get < unsigned
	int > ("run.num_days") << ","
						   << population_size << ","
						   << pt_config.get<double>("run.seeding_rate") << ","
						   << pt_config.get<double>("run.r0") << ","
						   << "NA" << "," // << pt_config.get<double>("run.transmission_rate") << ";"
						   << pt_config.get<double>("run.immunity_rate") << ","
						   << num_threads << ","
						   << pt_config.get < unsigned
	int > ("run.rng_seed") << ","
						   << run_time << "," << total_time << "," << num_cases << ","
						   << static_cast<double>(num_cases) / population_size << endl;
}

}
}

