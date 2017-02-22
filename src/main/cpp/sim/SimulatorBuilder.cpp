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
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of the Simulator class.
 */

#include "SimulatorBuilder.h"

#include "calendar/Calendar.h"
#include "core/Cluster.h"
#include "core/ClusterType.h"
#include "core/ContactProfile.h"
#include "core/Infector.h"
#include "core/LogMode.h"
#include "pop/Population.h"
#include "pop/PopulationBuilder.h"
#include "util/InstallDirs.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace stride::util;

shared_ptr<Simulator> SimulatorBuilder::Build(const string& config_file_name,
        unsigned int num_threads, bool track_index_case)
{
        // Configuration file.
        ptree pt_config;
        const auto file_path = InstallDirs::GetCurrentDir() /= config_file_name;
        if ( !is_regular_file(file_path) ) {
                throw runtime_error(string(__func__)
                        + ">Config file " + file_path.string() + " not present. Aborting.");
        }
        read_xml(file_path.string(), pt_config);

        // Done.
        return Build(pt_config, num_threads, track_index_case);
}

shared_ptr<Simulator> SimulatorBuilder::Build(const ptree& pt_config,
        unsigned int num_threads, bool track_index_case)
{
        // Disease file.
        ptree pt_disease;
        const auto file_name_d { pt_config.get<string>("run.disease_config_file") };
        const auto file_path_d { InstallDirs::GetDataDir() /= file_name_d };
        if ( !is_regular_file(file_path_d) ) {
                throw runtime_error(std::string(__func__)  + "> No file " + file_path_d.string());
        }
        read_xml(file_path_d.string(), pt_disease);

        // Contact file.
        ptree pt_contact;
        const auto file_name_c { pt_config.get("run.age_contact_matrix_file", "contact_matrix.xml") };
        const auto file_path_c { InstallDirs::GetDataDir() /= file_name_c };
        if ( !is_regular_file(file_path_c) ) {
                throw runtime_error(string(__func__)  + "> No file " + file_path_c.string());
        }
        read_xml(file_path_c.string(), pt_contact);

        // Done.
        return Build(pt_config, pt_disease, pt_contact, num_threads, track_index_case);
}

shared_ptr<Simulator> SimulatorBuilder::Build(const ptree& pt_config,
        const ptree& pt_disease, const ptree& pt_contact, unsigned int number_of_threads, bool track_index_case)
{
        auto sim = make_shared<Simulator>();

        // Initialize config ptree.
        sim->m_config_pt = pt_config;

        // Initialize track_index_case policy
        sim->m_track_index_case = track_index_case;

        // Initialize number of threads.
        sim->m_num_threads = number_of_threads;

        // Initialize calendar.
        sim->m_calendar = make_shared<Calendar>(pt_config);

        // Get log level.
        const string l = pt_config.get<string>("run.log_level", "None");
        sim->m_log_level = IsLogMode(l) ? ToLogMode(l) : throw runtime_error(string(__func__) + "> Invalid input for LogMode.");

        // Rng's.
        const auto seed = pt_config.get<double>("run.rng_seed");
        Random rng(seed);

        // Build population.
        sim->m_population = PopulationBuilder::Build(pt_config, pt_disease, rng);

        // Initialize clusters.
        InitializeClusters(sim);

        // Initialize disease profile.
        sim->m_disease_profile.Initialize(pt_config, pt_disease);

        // Initialize Rng handlers
        unsigned int new_seed = rng(numeric_limits<unsigned int>::max());
        for (size_t i = 0; i < sim->m_num_threads; i++) {
                sim->m_rng_handler.emplace_back(RngHandler(new_seed, sim->m_num_threads, i));
        }

        // Initialize contact profiles.
        Cluster::AddContactProfile(ClusterType::Household,     ContactProfile(ClusterType::Household, pt_contact));
        Cluster::AddContactProfile(ClusterType::School,        ContactProfile(ClusterType::School, pt_contact));
        Cluster::AddContactProfile(ClusterType::Work,          ContactProfile(ClusterType::Work, pt_contact));
        Cluster::AddContactProfile(ClusterType::PrimaryCommunity,  ContactProfile(ClusterType::PrimaryCommunity, pt_contact));
        Cluster::AddContactProfile(ClusterType::SecondaryCommunity,   ContactProfile(ClusterType::SecondaryCommunity, pt_contact));

        // Done.
        return sim;
}

void SimulatorBuilder::InitializeClusters(shared_ptr<Simulator> sim)
{
	// Determine number of clusters.
	unsigned int max_id_households           = 0U;
	unsigned int max_id_school_clusters      = 0U;
	unsigned int max_id_work_clusters        = 0U;
	unsigned int max_id_primary_community    = 0U;
	unsigned int max_id_secondary_community  = 0U;
	Population& population                   = *sim->m_population;

	for (const auto& p : population) {
		max_id_households            = std::max(max_id_households,          p.GetClusterId(ClusterType::Household));
		max_id_school_clusters       = std::max(max_id_school_clusters,     p.GetClusterId(ClusterType::School));
		max_id_work_clusters         = std::max(max_id_work_clusters,       p.GetClusterId(ClusterType::Work));
		max_id_primary_community     = std::max(max_id_primary_community,   p.GetClusterId(ClusterType::PrimaryCommunity));
		max_id_secondary_community   = std::max(max_id_secondary_community, p.GetClusterId(ClusterType::SecondaryCommunity));

	}

	// Keep separate id counter to provide a unique id for every cluster.
	unsigned int cluster_id = 1;

	for (size_t i = 0; i <= max_id_households; i++) {
		sim->m_households.emplace_back(Cluster(cluster_id, ClusterType::Household));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_school_clusters; i++) {
		sim->m_school_clusters.emplace_back(Cluster(cluster_id, ClusterType::School));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_work_clusters; i++) {
		sim->m_work_clusters.emplace_back(Cluster(cluster_id, ClusterType::Work));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_primary_community; i++) {
		sim->m_primary_community.emplace_back(Cluster(cluster_id, ClusterType::PrimaryCommunity));
		cluster_id++;
	}
	for (size_t i = 0; i <= max_id_secondary_community; i++) {
		sim->m_secondary_community.emplace_back(Cluster(cluster_id, ClusterType::SecondaryCommunity));
		cluster_id++;
	}

	// Cluster id '0' means "not present in any cluster of that type".
	for (auto& p: population) {
	        const auto hh_id = p.GetClusterId(ClusterType::Household);
		if (hh_id > 0) {
		        sim->m_households[hh_id].AddPerson(&p);
		}
		const auto sc_id = p.GetClusterId(ClusterType::School);
		if (sc_id > 0) {
				sim->m_school_clusters[sc_id].AddPerson(&p);
		}
		const auto wo_id = p.GetClusterId(ClusterType::Work);
		if (wo_id > 0) {
				sim->m_work_clusters[wo_id].AddPerson(&p);
		}
		const auto primCom_id = p.GetClusterId(ClusterType::PrimaryCommunity);
		if (primCom_id > 0) {
		        sim->m_primary_community[primCom_id].AddPerson(&p);
		}
		const auto secCom_id = p.GetClusterId(ClusterType::SecondaryCommunity);
		if (secCom_id > 0) {
		        sim->m_secondary_community[secCom_id].AddPerson(&p);
		}
	}
}

} // end_of_namespace
