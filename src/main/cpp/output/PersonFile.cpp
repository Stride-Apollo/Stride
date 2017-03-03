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
 * Implementation of the CasesFile class.
 */

#include "PersonFile.h"

#include "core/Health.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


namespace stride {
namespace output {

using namespace std;

PersonFile::PersonFile(const std::string& file)
{
	initialize(file);
}

PersonFile::~PersonFile()
{
	m_fstream.close();
}

void PersonFile::initialize(const std::string& file)
{
	m_fstream.open((file + "_person.csv").c_str());

	// add header
	m_fstream << "id,is_recovered,is_immune,start_infectiousness;"
			<< "end_infectiousness,start_symptomatic,end_symptomatic" << endl;
}

void PersonFile::print(const std::shared_ptr<const Population> population)
{
	for(const auto& p : *population) {
	        const auto& h = p.getHealth();
		if ( !h.isSusceptible() ) {
			m_fstream << p.getId() << ","  << h.isRecovered() << "," << h.isImmune() << ","
			        << h.getStartInfectiousness() << "," << h.getEndInfectiousness() << ","
			        << h.getStartSymptomatic() << "," << h.getEndSymptomatic()  << endl;
		}
	}
}

} // end_of_namespace
} // end_of_namespace
