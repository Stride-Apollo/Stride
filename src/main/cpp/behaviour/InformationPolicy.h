/*
 * InformationPolicy.h
 *
 *  Created on: Apr 13, 2017
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_BEHAVIOUR_INFORMATIONPOLICY_H_
#define SRC_MAIN_CPP_BEHAVIOUR_INFORMATIONPOLICY_H_

#include <string>

namespace stride {

/**
* Enum specifiying the information policy used for behaviour
*/
enum class InformationPolicy {Global = 0U, Local = 1U, Null};

/// Converts an InformationPolicy value to corresponding name.
std::string ToString(InformationPolicy w);

/// Check whether string is name of InformationPolicy value.
bool IsInformationPolicy(const std::string& s);

/// Converts a string with name to InformationPolicy value.
InformationPolicy ToInformationPolicy(const std::string& s);

} // end_of_namespace



#endif /* SRC_MAIN_CPP_BEHAVIOUR_INFORMATIONPOLICY_H_ */
