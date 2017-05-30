#pragma once

#include <utility>
#include <functional>
#include <type_traits>

#include "utils.h"

namespace unipar {
namespace internal {

template <typename Impl, typename... Types>
struct ResourceManager;

template <typename Impl, typename Type, typename... Rest>
struct ResourceManager<Impl, Type, Rest...> {
	using RestType = typename Impl::template RMType<Impl, Rest...>;
	using FuncType = std::function<Type()>;
	
	ResourceManager() = default;
	
	ResourceManager(const RestType& rest): m_rest(rest) {}
	
	ResourceManager(const ResourceManager<Impl, Type, Rest...>& rm)
			: m_rest(rm.m_rest) {
		using ThisRMType = typename Impl::template RMType<Impl, Type, Rest...>;
		((ThisRMType*) this)->setFunc(rm.m_func);
	}
	
	ResourceManager<Impl, Type, Rest...>& operator=(const ResourceManager<Impl, Type, Rest...>& other) {
		using ThisRMType = typename Impl::template RMType<Impl, Type, Rest...>;
		m_rest = other.m_rest;
		((ThisRMType*) this)->setFunc(other.m_func);
		return *this;
	}
	
	void setFunc(const FuncType& f) {
		m_func = f;
	}

	RestType& rest() {
		return m_rest;
	}
	
	const RestType& rest() const {
		return m_rest;
	}
	
	// The function call is essential and should be provided by
	// all classes inheriting this class!
	
protected:
	FuncType m_func;
	RestType m_rest;
};

template <typename Impl>
struct ResourceManager<Impl> {
	// End of the recursion
	template <typename F, typename... Args>
	typename std::result_of<F(Args...)>::type call(const F& func, Args&&... args) {
		return func(std::forward<Args>(args)...);
	}
};


template <typename Impl, typename... Types>
class ParallelWrapper {
public:
	using RMType = typename Impl::template RMType<Impl, Types...>;

	// Constructors .......................................
	
	ParallelWrapper(const Impl& impl): m_impl(impl) {}
	
	// Prefer the one without an argument.
	ParallelWrapper(int nthreads): m_impl(nthreads) { m_impl.init(m_resource_manager); }
	ParallelWrapper(): m_impl() { m_impl.init(m_resource_manager); }
	
	// Kinda private, however friend'ing is rather hard with variadic templates
	template <typename PrevResMan>
	ParallelWrapper(Impl& impl, PrevResMan& prev_rm)
			: m_impl(impl), m_resource_manager(prev_rm) {
		m_impl.init(m_resource_manager);
	}
	
	
	// Actual parallel constructions ......................
	
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		for_(first, last, typename unipar::utils::largest<IndexF, IndexL>::type(1), f);
	}
	
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		m_impl.parallelFor(first, last, step, f, m_resource_manager);
	}
	
	
	// Resource management ................................
	
	template <typename T>
	ParallelWrapper<Impl, T, Types...> withFunc(const std::function<T()>& f) {
		auto p = ParallelWrapper<Impl, T, Types...>(m_impl, m_resource_manager);
		p.resources().setFunc(f);
		return p;
	}
	
	// for Lambda's 
	template <typename T, typename Func>
	ParallelWrapper<Impl, T, Types...> withFunc(const Func& f) {
		return withFunc(std::function<T()>(f));
	}
	
	template <typename T, typename... Args>
	ParallelWrapper<Impl, T, Types...> with(Args... args) {
		return withFunc<T>(std::function<T()>([=](){
			return T(args...);
		}));
	}

	RMType& resources() {
		return m_resource_manager;
	}
	
	
	// Proxying to the implementation .....................
	
	// Also see the constructors above
	
	Impl& impl() { return m_impl; }
	const Impl& impl() const { return m_impl; }
	
	// This number should be seen as a hint and not a hard limit.
	int getNumThreads() const { return m_impl.getNumThreads(); }
	void setNumThreads(int nthreads) { m_impl.setNumThreads(nthreads); }
	
protected:
	Impl m_impl;
	RMType m_resource_manager;
};

}
}

