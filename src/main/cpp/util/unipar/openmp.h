#pragma once

#include <vector>
#include "unipar.h"
#include "interface.h"
#include <omp.h>

namespace unipar {
namespace internal {

template <typename Impl, typename... Types>
class OpenmpResourceManager;

template <typename Impl, typename Type, typename... Rest>
class OpenmpResourceManager<Impl, Type, Rest...> : public ResourceManager<Impl, Type, Rest...> {
public:
	using RestType = typename Impl::template RMType<Impl, Rest...>;
	using FuncType = std::function<Type()>;
	
	// Copy constructor does NOT copy the values!
	using ResourceManager<Impl, Type, Rest...>::ResourceManager;
	
	template <typename F, typename... Args>
	typename std::result_of<F(Type&, Args...)>::type call(const F& to_call, Args&&... args) {
		auto& value = m_values[omp_get_thread_num()];
		if (value == nullptr) {
			value = new Type(std::move(this->m_func()));
		}
		return this->m_rest.call(to_call, *value, std::forward<Args>(args)...);
	}
	
	void init(size_t size) {
		m_values.resize(size, nullptr);
		this->m_rest.init(size);
	}
	
	~OpenmpResourceManager() {
		for (auto& v: m_values) {
			if (v) {
				delete v;
				v = nullptr;
			}
		}
	}

protected:
	std::vector<Type*> m_values;
};

template <typename Impl>
class OpenmpResourceManager<Impl> : public ResourceManager<Impl> {
public:
	void init(size_t) {}
};

class _OpenmpParallel: public ParallelInterface {
public:
	template <typename Impl, typename... Types>
	using RMType = OpenmpResourceManager<Impl, Types...>;
	
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
	inline void setNumThreads(int nthreads) { m_nthreads = nthreads; }
	
private:
	int m_nthreads;
};

}

using OpenmpParallel = internal::ParallelWrapper<internal::_OpenmpParallel>;

}
