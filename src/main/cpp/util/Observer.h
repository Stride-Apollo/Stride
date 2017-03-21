#pragma once

/**
 * @file
 * Interface/Implementation of Observer.
 */

namespace stride {
namespace util {

template<typename E>
class Observer {
public:
	virtual void update(const E& subject) = 0;
};

}
}