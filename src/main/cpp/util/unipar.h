#pragma once

namespace stride {

#include "unipar/pick_choices.h"

#ifndef UNIPAR_IMPL
#define UNIPAR_IMPL UNIPAR_DUMMY
#endif

// We should have already picked one
//#define UNIPAR_IMPL UNIPAR_OPENMP
#include "unipar/pick.h"

}
