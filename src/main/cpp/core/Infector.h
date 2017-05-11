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

#include "behaviour/InformationPolicy.h"
#include "core/DiseaseProfile.h"
#include "core/LogMode.h"

#include <memory>

namespace stride {

class Cluster;
class Calendar;
namespace util { class Random; }

/**
 * Actual contacts and transmission in cluster (primary template).
 */
template<LogMode log_level, bool track_index_case, InformationPolicy information_policy>
class Infector {
public:
	static void execute(Cluster& cluster, DiseaseProfile disease_profile,
						util::Random& contact_handler, std::shared_ptr<const Calendar> sim_state);
};

/**
 * Actual contacts and transmissions in cluster (specialization for 'local' information policy).
 */
template<LogMode log_level, bool track_index_case>
class Infector<log_level, track_index_case, InformationPolicy::Local> {
public:
	static void execute(Cluster& cluster, DiseaseProfile disease_profile,
						util::Random& contact_handler, std::shared_ptr<const Calendar> calendar);
};

/**
 * Actual contacts and transmission in cluster (specialisation for logging all contacts).
 */
template<bool track_index_case>
class Infector<LogMode::Contacts, track_index_case, InformationPolicy::Global> {
public:
	static void execute(Cluster& cluster, DiseaseProfile disease_profile,
						util::Random& contact_handler, std::shared_ptr<const Calendar> calendar);
};

/// Explicit instantiation in cpp file.
extern template
class Infector<LogMode::None, false, InformationPolicy::Global>;
extern template class Infector<LogMode::None, false, InformationPolicy::Local>;

/// Explicit instantiation in cpp file.
extern template
class Infector<LogMode::None, true, InformationPolicy::Global>;
extern template class Infector<LogMode::None, true, InformationPolicy::Local>;

/// Explicit instantiation in cpp file.
extern template
class Infector<LogMode::Transmissions, false, InformationPolicy::Global>;
extern template class Infector<LogMode::Transmissions, true, InformationPolicy::Local>;

/// Explicit instantiation in cpp file.
extern template
class Infector<LogMode::Transmissions, true, InformationPolicy::Global>;
extern template class Infector<LogMode::Transmissions, true, InformationPolicy::Local>;

/// Explicit instantiation in cpp file.
extern template
class Infector<LogMode::Contacts, false, InformationPolicy::Global>;
extern template class Infector<LogMode::Contacts, false, InformationPolicy::Local>;

/// Explicit instantiation in cpp file.
extern template
class Infector<LogMode::Contacts, true, InformationPolicy::Global>;
extern template class Infector<LogMode::Contacts, true, InformationPolicy::Local>;

}

