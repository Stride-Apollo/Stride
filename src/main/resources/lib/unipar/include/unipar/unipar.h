#pragma once

#include <utility>
#include <functional>

#include "utils.h"

namespace unipar {
namespace internal {

template <typename _Type, typename _Func>
struct TypeFunc {
	using Type = _Type;
	using Func = _Func;
	Func func;
	
	static_assert(std::is_convertible<decltype(func()), Type>::value,
				  "The function has to return a type castable to the specified type");
	
	TypeFunc(const Func& _func): func(_func) {}
	TypeFunc() = default;
};


template <typename Impl, typename... TFs>
struct ResourceManager;

template <typename Impl, typename TF, typename... Rest>
struct ResourceManager<Impl, TF, Rest...> {
	
	ResourceManager(TF _tf, Rest&&... rest_tf)
		: tf(_tf), rest(std::forward<Rest>(rest_tf)...) {}
	
	ResourceManager(TF _tf, const typename Impl::template RMType<Impl, Rest...>& _rest)
		: tf(_tf), rest(_rest) {}

	ResourceManager() = default;

	void setFunc(const typename TF::Func& f) {
		tf.func = f;
	}

	auto& next() {
		return rest;
	}

protected:
	TF tf;
	typename Impl::template RMType<Impl, Rest...> rest;
};

template <typename Impl>
struct ResourceManager<Impl> {
	// End of the recursion
	template <typename F, typename... Args>
	auto call(const F& func, Args&&... args) {
		return func(std::forward<Args>(args)...);
	}
};


template <typename Impl, typename... TFs>
class ParallelWrapper {
public:

	// Constructors .......................................
	
	ParallelWrapper(const Impl& impl): m_impl(impl) {}
	
	// Prefer the one without an argument.
	ParallelWrapper(int nthreads): m_impl(nthreads) { m_impl.init(m_resource_manager); }
	ParallelWrapper(): m_impl() { m_impl.init(m_resource_manager); }
	
	// Kinda private, however friend'ing is rather hard with variadic templates
	template <typename PrevResMan, typename newTF>
	ParallelWrapper(Impl& impl, const PrevResMan& rm, newTF&& tf)
			: m_impl(impl), m_resource_manager(std::forward<newTF>(tf), rm) {
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
	
	template <typename T, typename Func>
	auto withFunc(Func f) {
		using TF = TypeFunc<T, decltype(f)>;
		return ParallelWrapper<Impl, TF, TFs...>(m_impl, m_resource_manager, TF(f));
	}

	template <typename T>
	auto withFunc() {
		using F = std::function<T()>;
		return withFunc<T, F>(F());
	}

	template <typename T, typename... Args>
	auto with(Args... args) {
		return withFunc<T>([=](){
			return T(args...);
		});
	}

	auto& getResourceManager() {
		return m_resource_manager;
	}
	
	// Proxying to the implementation .....................
	
	Impl& impl() { return m_impl; };
	const Impl& impl() const { return m_impl; };
	
	// This number should be seen as a hint and not a hard limit.
	int getNumThreads() const { return m_impl.getNumThreads(); }
	void setNumThreads(int nthreads) { m_impl.setNumThreads(nthreads); }
	
protected:
	Impl m_impl;
	typename Impl::template RMType<Impl, TFs...> m_resource_manager;
};

}
}

