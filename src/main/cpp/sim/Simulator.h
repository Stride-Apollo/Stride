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
 * Header for the Simulator class.
 */

//#include "core/Cluster.h"
#include "core/DiseaseProfile.h"
#include "core/LogMode.h"
#include "core/RngHandler.h"
#include "behavior/behavior_policies/NoBehavior.h"
#include "pop/Person.h"
#include "pop/Traveller.h"
#include "util/Subject.h"
#include "util/Random.h"
#include "util/unipar.h"

#include "behavior/belief_policies/NoBelief.h"
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace stride {

class Population;
class Calendar;
class Cluster;

/**
 * Main class that contains and direct the virtual world.
 */
class Simulator : public util::Subject<Simulator> {
public:
	using PersonType = Person<NoBehavior, NoBelief>;
	using TravellerType = Traveller<PersonType>;

	// Default constructor for empty Simulator.
	Simulator();

	/// Get the population.
	const std::shared_ptr<const Population> getPopulation() const;

	/// Change track_index_case setting.
	void setTrackIndexCase(bool track_index_case);

	/// Run one time step, computing full simulation (default) or only index case.
	void timeStep();

	void host(const std::vector<Simulator::TravellerType>& travellers);

private:
	// Information about travellers
	// original ID ->
	std::map<unsigned int, Simulator::TravellerType> m_trav_elsewhere;
	std::map<unsigned int, Simulator::TravellerType> m_trav_hosting;

private:
	/// Update the contacts in the given clusters.
	template<LogMode log_level, bool track_index_case = false>
	void updateClusters();

private:
	boost::property_tree::ptree m_config_pt;            ///< Configuration property tree.

private:

	unsigned int m_num_threads; 			///< The number of threads (as a hint)
	decltype(Parallel().withFunc<RngHandler>()) m_parallel;
	unsigned int m_seed;
	std::shared_ptr<util::Random> m_seed_rng;

	// Maintaining the RNG is only possible in single-threaded mode
	// TODO_UNIPAR
	//std::unique_ptr<RngHandler> m_rng_handler;  ///< Pointer to the RngHandler

	LogMode m_log_level;            		///< Specifies logging mode.
	std::shared_ptr<Calendar> m_calendar;	///< Management of calendar.

private:
	std::shared_ptr<Population> m_population;	 ///< Pointer to the Population.

	std::vector<Cluster> m_households;           ///< Container with household Clusters.
	std::vector<Cluster> m_school_clusters;      ///< Container with school Clusters.
	std::vector<Cluster> m_work_clusters;        ///< Container with work Clusters.
	std::vector<Cluster> m_primary_community;    ///< Container with primary community Clusters.
	std::vector<Cluster> m_secondary_community;  ///< Container with secondary community  Clusters.

	DiseaseProfile m_disease_profile;      ///< Profile of disease.

	bool m_track_index_case;     ///< General simulation or tracking index case.

private:
	friend class SimulatorBuilder;
};


}
