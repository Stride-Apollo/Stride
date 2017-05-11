#pragma once

namespace stride {
namespace util {

// Like golang's 'defer' statement

template <typename Func>
class Defer {
public:
	Defer(const Func& f): m_func(f) {}
	~Defer() { m_func(); }
private:
	const Func& m_func;
};

template <typename Func>
Defer<Func> _defer(const Func& f) {
	return Defer<Func>(f);
}

#define defer(s) auto _defer_##__FILE__##_##__LINE__ = stride::util::_defer([&](){ s ; });

}
}
