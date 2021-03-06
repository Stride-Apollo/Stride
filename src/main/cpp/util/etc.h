#pragma once

#include <memory>

namespace stride {
namespace util {

// Like golang's 'defer' statement

template<typename Func>
class Defer {
public:
	Defer(const Func& f) : m_func(f) {}

	~Defer() { m_func(); }

private:
	const Func& m_func;
};

template<typename Func>
Defer<Func> _defer(const Func& f) {
	return Defer<Func>(f);
}

#define __defer_name(name) _defer_ ## name
#define __defer(line, s) auto __defer_name(line) = stride::util::_defer([&](){ s ; });
#define defer(s) __defer(__LINE__, s)

}
}


#if __cplusplus <= 201103L

namespace std {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&& ... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}

#endif
