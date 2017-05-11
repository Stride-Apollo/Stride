#pragma once

#include "unipar.h"
#include "interface.h"

#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/enumerable_thread_specific.h"

#include <iostream>

namespace unipar {
namespace internal {

template <typename Impl, typename... TFs>
struct TbbResourceManager;

template <typename Impl, typename TF, typename... Rest>
struct TbbResourceManager<Impl, TF, Rest...> : public ResourceManager<Impl, TF, Rest...> {
	using TLSType = tbb::enumerable_thread_specific<typename TF::Type>;
	TLSType tls;

	// Watch out, tls has to be set!
	TbbResourceManager() {
		std::cout << this << " Tbb Resource Manager with empty tls init...\n";
	}

	TbbResourceManager(TF _tf, Rest&&... rest_tf)
		: ResourceManager<Impl, TF, Rest...>(_tf, std::forward<Rest>(rest_tf)...), tls(this->tf.func) {}
	
	TbbResourceManager(TF _tf, const typename Impl::template RMType<Impl, Rest...>& _rest)
		: ResourceManager<Impl, TF, Rest...>(_tf, _rest), tls(this->tf.func) {}
	
	template <typename F, typename... Args>
	auto call(const F& func, Args&&... args) {
		//std::cout << this << " Current values: \n";
		//for (auto& i: tls) {
		//	std::cout << "  - " << i.get() << "\n";
		//}
		auto val = std::move(tls.local());
		//std::cout << this << " Value requested... " << val.get() << "\n";
		this->rest.call(func, std::forward<typename TLSType::reference>(val), std::forward<Args>(args)...);
		//std::cout << this << " Value is now... " << val.get() << "\n";
	}

	void setFunc(const typename TF::Func& f) {
		this->tf.func = f;
		this->tls = TLSType(f);
	}
};

template <typename Impl>
struct TbbResourceManager<Impl> : public ResourceManager<Impl> {};


class _TbbParallel : public ParallelInterface {
public:
	template <typename Impl, typename... TFs>
	using RMType = TbbResourceManager<Impl, TFs...>;
	
	_TbbParallel(int nthreads = -1): m_nthreads(nthreads) {}
	
	// TBB dislikes manually setting the amount of threads a LOT. In order to not introduce
	// unexpected behaviour, we can't use task_scheduler_init for this instance, we need to
	// create one for this call separately.
	// In order to avoid this overhead, don't manually set the amount of threads or create
	// a tbb::task_scheduler_init at the start of your program.
	template <typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
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
