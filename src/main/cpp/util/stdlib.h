#pragma once

// This file contains a bunch of small additions to the standard library
// that should objectively always have been a part of C++11

#include <memory>

namespace std {

#if __cplusplus > 199711L

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args ) {
	return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

#endif

}
