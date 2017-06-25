#pragma once

#include <memory>
#include "unipar.h"
#include "interface.h"

namespace unipar {
namespace internal {

template<typename Impl, typename... Types>
class DummyResourceManager;

template<typename Impl, typename Type, typename... Rest>
class DummyResourceManager<Impl, Type, Rest...> : public ResourceManager<Impl, Type, Rest...> {
public:
	using ResourceManager<Impl, Type, Rest...>::ResourceManager;

	template<typename F, typename... Args>
	typename std::result_of<F(Type&, Args...)>::type call(const F& func, Args&& ... args) {
		if (m_value == nullptr) {
			// Copy constructor is needed!
			m_value = new Type(std::move(this->m_func()));
		}
		return this->m_rest.call(func, *m_value, std::forward<Args>(args)...);
	}

	~DummyResourceManager() {
		if (m_value) {
			delete m_value;
			m_value = nullptr;
		}
	}

protected:
	Type* m_value = nullptr;
};

template<typename Impl>
class DummyResourceManager<Impl> : public ResourceManager<Impl> {
};


class _DummyParallel : public ParallelInterface {
public:
	template<typename Impl, typename... Types>
	using RMType = DummyResourceManager<Impl, Types...>;

	// The dummy implementation will obviously never use more than 1 thread
	// However, to remain compatible with otherwise multithreaded code, we allow
	// this constructor.
	_DummyParallel(int= 1) {}

	// Dummy non-multithreaded implementation
	template<typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
	void parallelFor(IndexF first, IndexL last, IndexS step, const Func& f, RM& rm) {
		for (IndexF i = first; i < last; i += step) {
			rm.call(f, i);
		}
	}

	inline int getNumThreads() const { return 1; }
};

}

using DummyParallel = internal::ParallelWrapper<internal::_DummyParallel>;

}
