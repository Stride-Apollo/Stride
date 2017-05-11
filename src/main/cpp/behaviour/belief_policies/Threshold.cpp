/*
 * Threshold.cpp
 *
 *  Created on: May 7, 2017
 *      Author: elise
 */

#include "Threshold.h"

namespace stride {

template class Threshold<true, false>;
template class Threshold<false, true>;
template class Threshold<true, true>;

} /* namespace stride */
