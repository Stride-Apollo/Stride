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
 * Implementation of InformationPolicy.
 */

#include "../behaviour/InformationPolicy.h"

#include <boost/algorithm/string.hpp>
#include <map>
#include <string>

namespace {

using stride::InformationPolicy;
using boost::to_upper;
using namespace std;

map<InformationPolicy, string> g_information_policy_name {
                make_pair(InformationPolicy::Global,    		"Global"),
                make_pair(InformationPolicy::Local,         	"Local"),
				make_pair(InformationPolicy::Null, 				"Null")
};

map<string, InformationPolicy> g_name_information_policy {
                make_pair("GLOBAL",   				InformationPolicy::Global),
                make_pair("LOCAL",        			InformationPolicy::Local),
				make_pair("NULL",					InformationPolicy::Null)
};

}

namespace stride {

string ToString(InformationPolicy w)
{
	return (g_information_policy_name.count(w) == 1) ? g_information_policy_name[w] : "Null";
}

bool IsInformationPolicy(const string& s)
{
        std::string t {s};
        to_upper(t);
        return (g_name_information_policy.count(t) == 1);
}

InformationPolicy ToInformationPolicy(const string& s)
{
        std::string t {s};
        to_upper(t);
	return (g_name_information_policy.count(t) == 1) ? g_name_information_policy[t] : InformationPolicy::Null;
}

} // namespace


