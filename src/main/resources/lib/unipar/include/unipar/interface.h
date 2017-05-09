#pragma once

#include <stdexcept>

namespace unipar {
namespace internal {

class ParallelInterface {
public:
	
	// template <typename Impl, typename... TFs>
	// using RMType = SomeResourceManager<Impl, TFs...>;
	
	ParallelInterface(int) {}
	ParallelInterface() {}
	
	template <typename RM>
	void init(RM& rm) {
		// Implementing this is optional
	}
	
	inline int getNumThreads() const {
		throw std::logic_error("Please implement getNumThreads");
	}

	inline void setNumThreads(int nthreads) {
		// Implementing this is optional
	}
	
	template <typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
	void parallelFor(IndexF first, IndexL last, IndexS step, const Func& f, RM& rm) {
		throw std::logic_error("Please implement parallelFor");
	}
};

}
}
