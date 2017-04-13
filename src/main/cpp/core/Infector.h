#ifndef INFECTOR_H_INCLUDED
#define INFECTOR_H_INCLUDED
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
class RngHandler;
class Calendar;

/**
 * Actual contacts and transmission in cluster (primary template).
 */
template<LogMode log_level, bool track_index_case, InformationPolicy information_policy>
class Infector
{
public:
	///
	static void Execute(Cluster& cluster, DiseaseProfile disease_profile,
	        RngHandler& contact_handler, std::shared_ptr<const Calendar> sim_state);
};

/**
 * Actual contacts and transmission in cluster (specialisation for logging all contacts).
 */
template<bool track_index_case, InformationPolicy information_policy>
class Infector<LogMode::Contacts, track_index_case, information_policy>
{
public:
        ///
        static void Execute(Cluster& cluster, DiseaseProfile disease_profile,
                RngHandler& contact_handler, std::shared_ptr<const Calendar> calendar);
};

/// Explicit instantiation in cpp file.
extern template class Infector<LogMode::None, false, InformationPolicy::None>;

/// Explicit instantiation in cpp file.
extern template class Infector<LogMode::None, true, InformationPolicy::None>;

/// Explicit instantiation in cpp file.
extern template class Infector<LogMode::Transmissions, false, InformationPolicy::None>;

/// Explicit instantiation in cpp file.
extern template class Infector<LogMode::Transmissions, true, InformationPolicy::None>;

/// Explicit instantiation in cpp file.
extern template class Infector<LogMode::Contacts, false, InformationPolicy::None>;

/// Explicit instantiation in cpp file.
extern template class Infector<LogMode::Contacts, true, InformationPolicy::None>;

} // end_of_namespace

#endif // include-guard
