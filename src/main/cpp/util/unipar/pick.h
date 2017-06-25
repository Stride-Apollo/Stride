#pragma once

// Usage: define the parameter UNIPAR_IMPL to one of these values:
//   1: UNIPAR_DUMMY (default) --> For the dummy implementation (without parallelisation)
//   2: UNIPAR_TBB             --> For Intel TBB
//   3: UNIPAR_OPENMP          --> For OpenMP

// Example:
//   #include "unipar/pick_choices.h"
//   #define UNIPAR_IMPL UNIPAR_TBB
//   #include "unipar/pick.h"

// After this, you can use the class `Parallel`.

#include "pick_choices.h"

#ifndef UNIPAR_IMPL
#define UNIPAR_IMPL DUMMY
#warning Using default Dummy Unipar Implementation
#endif

#if UNIPAR_IMPL == UNIPAR_DUMMY

#include "dummy.h"

namespace unipar {
using Parallel = unipar::DummyParallel;
}

#elif UNIPAR_IMPL == UNIPAR_TBB

#include "tbb.h"
namespace unipar {
	using Parallel = unipar::TbbParallel;
}

#elif UNIPAR_IMPL == UNIPAR_OPENMP

#include "openmp.h"
namespace unipar {
	using Parallel = unipar::OpenmpParallel;
}

#else

#error Unknown Unipar Implementation

#endif
