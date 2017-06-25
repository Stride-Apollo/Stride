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
 * Header for the Infector class.
 */

#include "behaviour/information_policies/NoLocalInformation.h"
#include "behaviour/information_policies/LocalDiscussion.h"

#include "core/DiseaseProfile.h"
#include "core/LogMode.h"

#include <memory>
#include <spdlog/logger.h>

namespace stride {

class Cluster;

class Calendar;
namespace util { class Random; }

/**
 * Actual contacts and transmission in cluster (primary template).
 */
template<LogMode log_level, bool track_index_case, typename local_information_policy>
class Infector {
public:
	static void execute(Cluster& cluster, DiseaseProfile disease_profile,
						util::Random& contact_handler, std::shared_ptr<const Calendar> calendar,
						spdlog::logger& logger);
};

/**
 * Actual contacts and transmissions in cluster (specialization for NoLocalInformation policy)
 */
template<LogMode log_level, bool track_index_case>
class Infector<log_level, track_index_case, NoLocalInformation> {
public:
	static void execute(Cluster& cluster, DiseaseProfile disease_profile,
						util::Random& contact_handler, std::shared_ptr<const Calendar> calendar,
						spdlog::logger& logger);
};

/**
 * Actual contacts and transmission in cluster (specialization for logging all contacts, and with NoLocalInformation policy).
 */
template<bool track_index_case>
class Infector<LogMode::Contacts, track_index_case, NoLocalInformation> {
public:
	static void execute(Cluster& cluster, DiseaseProfile disease_profile,
						util::Random& contact_handler, std::shared_ptr<const Calendar> calendar,
						spdlog::logger& logger);
};


/// Explicit instantiations in cpp file.
extern template
class Infector<LogMode::None, false, NoLocalInformation>;

extern template
class Infector<LogMode::None, false, LocalDiscussion<Simulator::PersonType>>;

extern template
class Infector<LogMode::None, true, NoLocalInformation>;

extern template
class Infector<LogMode::None, true, LocalDiscussion<Simulator::PersonType>>;

extern template
class Infector<LogMode::Transmissions, false, NoLocalInformation>;

extern template
class Infector<LogMode::Transmissions, false, LocalDiscussion<Simulator::PersonType>>;

extern template
class Infector<LogMode::Transmissions, true, NoLocalInformation>;

extern template
class Infector<LogMode::Transmissions, true, LocalDiscussion<Simulator::PersonType>>;

extern template
class Infector<LogMode::Contacts, false, NoLocalInformation>;

extern template
class Infector<LogMode::Contacts, false, LocalDiscussion<Simulator::PersonType>>;

extern template
class Infector<LogMode::Contacts, true, NoLocalInformation>;

extern template
class Infector<LogMode::Contacts, true, LocalDiscussion<Simulator::PersonType>>;


}
