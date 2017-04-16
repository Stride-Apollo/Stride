#pragma once
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
 * Header for the SimulatorBuilder class.
 */

#include "Simulator.h"
#include "core/ClusterType.h"
#include "util/GeoCoordinate.h"

#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <map>
#include <utility>

namespace stride {

class Population;

class Calendar;

/**
 * Main class that contains and direct the virtual world.
 */
class SimulatorBuilder {
public:
	/// Build simulator.
	static std::shared_ptr<Simulator> build(
			const std::string& config_file_name,
			unsigned int num_threads = 1U,
			bool track_index_case = false);

	/// Build simulator.
	static std::shared_ptr<Simulator> build(
			const boost::property_tree::ptree& pt_config,
			unsigned int num_threads = 1U,
			bool track_index_case = false);

	/// Build simulator.
	static std::shared_ptr<Simulator> build(
			const boost::property_tree::ptree& pt_config,
			const boost::property_tree::ptree& pt_disease,
			const boost::property_tree::ptree& pt_contact,
			unsigned int number_of_threads = 1U,
			bool track_index_case = false);

private:
	/// Initialize the clusters.
	static void initializeClusters(
			std::shared_ptr<Simulator> sim,
			const boost::property_tree::ptree& pt_config);

	/// Initialize the districts, duplicate city names are ignored (only the first occurrence is counted)
	static void initializeDistricts(
			std::shared_ptr<Simulator> sim,
			const boost::property_tree::ptree& pt_config);

	/// Initialize the locations (read the from the given file) and return them
	/// If the filename is "", it will assume that you use an older version of stride which has no locations, all locations will be in the origin (0,0)
	/// Unreadable input will result in zeroes/ClusterType::Null
	static std::map<std::pair<ClusterType, uint>, util::GeoCoordinate> initializeLocations(std::string filename);
};

}

