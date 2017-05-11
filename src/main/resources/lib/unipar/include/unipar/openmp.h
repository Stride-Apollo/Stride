#pragma once

#include <vector>
#include "unipar.h"
#include "interface.h"
#include <omp.h>

namespace unipar {
namespace internal {

template <typename Impl, typename... TFs>
struct OpenmpResourceManager;

template <typename Impl, typename TF, typename... Rest>
struct OpenmpResourceManager<Impl, TF, Rest...> : public ResourceManager<Impl, TF, Rest...> {
	std::vector<typename TF::Type*> values;
	
	using ResourceManager<Impl, TF, Rest...>::ResourceManager;
	
	template <typename F, typename... Args>
	auto call(const F& func, Args&&... args) {
		auto& value = values[omp_get_thread_num()];
		if (value == nullptr) {
			// Copy constructor is needed!
			value = new typename TF::Type(this->tf.func());
		}
		return this->rest.call(func, *value, std::forward<Args>(args)...);
	}
	
	void init(size_t size) {
		values.resize(size, nullptr);
		this->rest.init(size);
	}
	
	~OpenmpResourceManager() {
		for (auto& v: values) {
			if (v) {
				delete v;
				v = nullptr;
			}
		}
	}
};

template <typename Impl>
struct OpenmpResourceManager<Impl> : public ResourceManager<Impl> {
	void init(size_t) {}
};

class _OpenmpParallel: public ParallelInterface {
public:
	template <typename Impl, typename... TFs>
	using RMType = OpenmpResourceManager<Impl, TFs...>;
	
	_OpenmpParallel() {
		#pragma omp parallel
		{
			m_nthreads = omp_get_num_threads();
		}
	}
	
	_OpenmpParallel(int nthreads): m_nthreads(nthreads) {}
	
	template <typename RM>
	void init(RM& rm) {
		rm.init(m_nthreads);
	}
	
	template <typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
	void parallelFor(IndexF first, IndexL last, IndexS step, const Func& f, RM& rm) {
		#pragma omp parallel for num_threads(m_nthreads) schedule(runtime)
		for (IndexF i=first; i < last; i += step) {
			rm.call(f, i);
		}
	}
	
	inline int getNumThreads() const { return m_nthreads; }
	inline int setNumThreads(int nthreads) { m_nthreads = nthreads; }
	
private:
	int m_nthreads;
};

}

using OpenmpParallel = internal::ParallelWrapper<internal::_OpenmpParallel>;

}
