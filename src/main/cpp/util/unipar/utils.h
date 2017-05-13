#pragma once

#include <utility>
#include <type_traits>

namespace unipar {
namespace utils {

// Taken from http://stackoverflow.com/a/1953137/2678118
template<bool, typename T1, typename T2>
struct is_cond {
	typedef T1 type;
};

template<typename T1, typename T2>
struct is_cond<false, T1, T2> {
	typedef T2 type;
};

template<typename T1, typename T2>
struct largest {
	typedef typename is_cond< (sizeof(T1)>sizeof(T2)), T1, T2>::type type;
};

template<typename T1, typename T2, typename T3>
struct largest3 {
	typedef typename is_cond< (sizeof(T1)>sizeof(T2)), T1, T2>::type _type_temp;
	typedef typename is_cond< (sizeof(_type_temp)>sizeof(T3)), _type_temp, T3>::type type;
};


}
}
