#pragma once

#include <memory>
#include "unipar.h"
#include "interface.h"

namespace unipar {
namespace internal {

template <typename Impl, typename... TFs>
struct DummyResourceManager;

template <typename Impl, typename TF, typename... Rest>
struct DummyResourceManager<Impl, TF, Rest...> : public ResourceManager<Impl, TF, Rest...> {
	typename TF::Type* value = nullptr;
	
	using ResourceManager<Impl, TF, Rest...>::ResourceManager;
	
	template <typename F, typename... Args>
	auto call(const F& func, Args&&... args) {
		if (value == nullptr) {
			// Copy constructor is needed!
			value = new typename TF::Type(this->tf.func());
		}
		return this->rest.call(func, *value, std::forward<Args>(args)...);
	}
	
	~DummyResourceManager() {
		if (value) {
			delete value;
			value = nullptr;
		}
	}
};

template <typename Impl>
struct DummyResourceManager<Impl> : public ResourceManager<Impl> {};


class _DummyParallel: public ParallelInterface {
public:
	template <typename Impl, typename... TFs>
	using RMType = DummyResourceManager<Impl, TFs...>;
	
	// The dummy implementation will obviously never use more than 1 thread
	// However, to remain compatible with otherwise multithreaded code, we allow
	// this constructor.
	_DummyParallel(int=1) {}
	
	// Dummy non-multithreaded implementation
	template <typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
	void parallelFor(IndexF first, IndexL last, IndexS step, const Func& f, RM& rm) {
		for (IndexF i=first; i < last; i += step) {
			rm.call(f, i);
		}
	}
	
	inline int getNumThreads() const { return 1; }
};

}

using DummyParallel = internal::ParallelWrapper<internal::_DummyParallel>;

}
