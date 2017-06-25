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

#include "behaviour/information_policies/InformationPolicy.h"
#include "behaviour/behaviour_policies/Vaccination.h"
#include "behaviour/information_policies/NoLocalInformation.h"
#include "behaviour/information_policies/NoGlobalInformation.h"
#include "behaviour/belief_policies/NoBelief.h"
#include "behaviour/behaviour_policies/NoBehaviour.h"

#include "core/DiseaseProfile.h"
#include "core/LogMode.h"
#include "core/District.h"
#include "core/ClusterType.h"
#include "pop/Person.h"
#include "pop/Traveller.h"
#include "util/Subject.h"
#include "util/Random.h"
#include "util/unipar.h"
#include "util/SimplePlanner.h"
#include "behaviour/belief_policies/NoBelief.h"
#include <boost/property_tree/ptree.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace stride {

class Population;
class Calendar;
class Cluster;
class AsyncSimulator;
using uint = unsigned int;
namespace run { class Runner; }

/**
 * Main class that contains and direct the virtual world.
 */
class Simulator : public Subject<Simulator> {
public:
	using GlobalInformationPolicy = NoGlobalInformation;
	using LocalInformationPolicy = NoLocalInformation;
	//using BeliefPolicy = Threshold<true, false>;
	using BeliefPolicy = NoBelief;
	//using BehaviourPolicy = Vaccination<BeliefPolicy>;
	using BehaviourPolicy = NoBehaviour<BeliefPolicy>;
	using PersonType = Person<BehaviourPolicy, BeliefPolicy>;
	//using LocalInformationPolicy = LocalDiscussion<PersonType>;
	using TravellerType = Traveller<PersonType>;

	/// Default constructor for empty Simulator.
	Simulator();

	/// Get the population.
	const std::shared_ptr<const Population> getPopulation() const;

	/// Change track_index_case setting.
	void setTrackIndexCase(bool track_index_case);

	void setName(string name) { m_name = name; }

	string getName() const { return m_name; }

	void steCommunicationMap(const std::map<string, AsyncSimulator*>& comm_map) {m_communication_map = comm_map;}

	/// Run one time step, computing full simulation (default) or only index case.
	void timeStep();

	/// Return the calendar of this simulator
	const Calendar& getCalendar() const {return *m_calendar;}

	/// Get the clusters of this simulator based on the cluster type
	/// This is rather for testing purposes
	const std::vector<Cluster>& getClusters(ClusterType cluster_type) const;

	/// Retrieve the states of the rng's
	std::vector<std::string> getRngStates() const;

	/// Set the states of the rng's
	void setRngStates(std::vector<std::string> states);

	/// Return an index to a cluster in the given vector
	/// Current policy: search for the first cluster with equal coordinates
	/// Return the size of the vector if you can't find any
	uint chooseCluster(const GeoCoordinate& coordinate, const vector<Cluster>& clusters, double influence);

	/// Receive travellers
	/// @argument travellers: the travellers this simulator has to host. Contains the data needed to identify a person in the home simulator
	/// @argument days: The amount of days the travellers will stay in this simulator
	/// @argument destination_district: The name of the city in which the airport / facility is located e.g. "Antwerp"
	/// @argument destination_facility: The name of the facility / airport e.g. "ANR"
	/// TODO: future return value?
	bool hostForeignTravellers(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility);

	/// Return people that were abroad
	/// @argument travellers_indices: contains the indices (in the m_population->m_original vector) of the returning people
	/// @argument health_status: The Health of the returning people (equal size as travellers_indices, health_status.at(i) belongs to travellers_indices.at(i))
	/// TODO: future return value?
	bool welcomeHomeTravellers(const vector<uint>& travellers_indices, const vector<Health>& health_status);

	/// Return people that are here FROM abroad
	void returnForeignTravellers();

	void sendNewTravellers(uint amount, uint days, const string& destination_sim_id, string destination_district, string destination_facility);

	const SimplePlanner<Traveller<Simulator::PersonType> >& getPlanner() const {return m_planner;}

private:
	// Information about travellers
	std::map<unsigned int, Simulator::TravellerType> m_trav_elsewhere;
	std::map<unsigned int, Simulator::TravellerType> m_trav_hosting;

private:
	/// Update the contacts in the given clusters.
	template<LogMode log_level, bool track_index_case = false>
	void updateClusters();

private:
	unsigned int                        m_num_threads;          ///< The number of threads(as a hint)

	#if UNIPAR_IMPL == UNIPAR_DUMMY
		using RandomRef = util::Random*;
	#else
		using RandomRef = std::unique_ptr<util::Random>;
	#endif

	decltype(Parallel().with<RandomRef>()) m_parallel;

	std::shared_ptr<util::Random> 		m_rng;
	LogMode                             m_log_level;            ///< Specifies logging mode.
	std::shared_ptr<Calendar>           m_calendar;             ///< Management of calendar.
public:	// TODO write getters or set friend class for ClusterSaver

	boost::property_tree::ptree m_config_pt;            ///< Configuration property tree.
	boost::property_tree::ptree m_config_pop;
	std::shared_ptr<spdlog::logger>		m_logger;
	std::shared_ptr<Population> m_population;	 ///< Pointer to the Population.

	std::vector<Cluster> m_households;           ///< Container with household Clusters.
	std::vector<Cluster> m_school_clusters;      ///< Container with school Clusters.
	std::vector<Cluster> m_work_clusters;        ///< Container with work Clusters.
	std::vector<Cluster> m_primary_community;    ///< Container with primary community Clusters.
	std::vector<Cluster> m_secondary_community;  ///< Container with secondary community Clusters.

	std::vector<District> m_districts;    ///< Container with districts (villages and cities).

	std::map<string, AsyncSimulator*> m_communication_map	;    ///< Communication between the simulator and the senders

	DiseaseProfile m_disease_profile;      ///< Profile of disease.

	bool m_track_index_case;     ///< General simulation or tracking index case.

	uint m_next_id;		///< The ID of the next traveller that arrives.
	uint m_next_hh_id;	///< The household ID of the next traveller that arrives.
	string m_name;	///< Name of the simulator (the region it simulates)

	SimplePlanner<Traveller<Simulator::PersonType> > m_planner;		///< The Planner, responsible for the timing of travellers (when do they return home?).

	friend class SimulatorBuilder;
	friend class LocalSimulatorAdapter;
	friend class Hdf5Saver;
	friend class Hdf5Loader;
	friend class Runner;
};

}
