#pragma once

#define UNIPAR_DUMMY 1
#define UNIPAR_TBB 2
#define UNIPAR_OPENMP 3


// Sometimes, it's more useful to work with existence of defines rather than their values

#ifdef UNIPAR_IMPL_DUMMY
#define UNIPAR_IMPL UNIPAR_DUMMY
#else
#ifdef UNIPAR_IMPL_TBB
#define UNIPAR_IMPL UNIPAR_TBB
#else
#ifdef UNIPAR_IMPL_OPENMP
#define UNIPAR_IMPL UNIPAR_OPENMP
#endif
#endif
#endif


// However, we like to maintain only one way of configuration, so past this point you
// can no longer use them.

#undef UNIPAR_IMPL_DUMMY
#undef UNIPAR_IMPL_OPENMP
#undef UNIPAR_IMPL_TBB
