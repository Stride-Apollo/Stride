#pragma once

#include "unipar.h"
#include "interface.h"

#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/enumerable_thread_specific.h"

#include <iostream>
#include <memory>

namespace unipar {
namespace internal {

template<typename Impl, typename... Types>
class TbbResourceManager;

template<typename Impl, typename Type, typename... Rest>
class TbbResourceManager<Impl, Type, Rest...> : public ResourceManager<Impl, Type, Rest...> {
public:
	using RestType = typename Impl::template RMType<Impl, Rest...>;
	using FuncType = std::function<Type()>;
	using TLSType = tbb::enumerable_thread_specific<Type>;

	using ResourceManager<Impl, Type, Rest...>::ResourceManager;

	template<typename F, typename... Args>
	typename std::result_of<F(Type&, Args...)>::type call(const F& func, Args&& ... args) {
		return this->m_rest.call(func, std::forward<Type&>(m_tls->local()), std::forward<Args>(args)...);
	}

	void setFunc(const FuncType& f) {
		this->m_func = f;
		this->m_tls.reset(new TLSType(f)); // sadly, I can't directly set the function
	}

protected:
	std::unique_ptr<TLSType> m_tls;
};

template<typename Impl>
class TbbResourceManager<Impl> : public ResourceManager<Impl> {
};


class _TbbParallel : public ParallelInterface {
public:
	template<typename Impl, typename... Types>
	using RMType = TbbResourceManager<Impl, Types...>;

	_TbbParallel(int nthreads = -1) : m_nthreads(nthreads) {}

	// TBB dislikes manually setting the amount of threads a LOT. In order to not introduce
	// unexpected behaviour, we can't use task_scheduler_init for this instance, we need to
	// create one for this call separately.
	// In order to avoid this overhead, don't manually set the amount of threads or create
	// a tbb::task_scheduler_init at the start of your program.
	template<typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
	void parallelFor(IndexF first, IndexL last, IndexS step, const Func& f, RM& rm) {
		using Largest = typename utils::largest3<IndexF, IndexL, IndexS>::type;
		auto wrapper = [&](Largest i) {
			return rm.call(f, i);
		};

		if (m_nthreads == -1) {
			tbb::parallel_for<Largest, decltype(wrapper)>(first, last, step, wrapper);
		} else {
			//std::cout << "Limiting to " << m_nthreads << "threads \n";
			tbb::task_scheduler_init init(m_nthreads);
			tbb::parallel_for<Largest, decltype(wrapper)>(first, last, step, wrapper);
		}
	}

	inline int getNumThreads() const {
		if (m_nthreads == -1) {
			return tbb::task_scheduler_init::default_num_threads();
		} else {
			return m_nthreads;
		}
	}

	inline void setNumThreads(int nthreads) { m_nthreads = nthreads; }

private:
	int m_nthreads;
};

}

using TbbParallel = internal::ParallelWrapper<internal::_TbbParallel>;

}
