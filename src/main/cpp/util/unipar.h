#pragma once
// This file is just to regulate the usage of unipar in the rest of the project.
// The files in util/unipar/ basically form their own library in their own namespace.

#include "unipar/pick_choices.h"

#ifndef UNIPAR_IMPL
#define UNIPAR_IMPL UNIPAR_DUMMY
#endif

// We should have already picked one
//#define UNIPAR_IMPL UNIPAR_OPENMP
#include "unipar/pick.h"

namespace stride {
	using Parallel = unipar::Parallel;
}
