/**
 * @file
 * Implementation of the Publisher class.
 */

#include "Publisher.h"
#include "Subscriber.h"

#include <set>

namespace stride {
namespace patterns {

using namespace std;

void Publisher::Listen(Subscriber* subscriber)
{
	m_subscribers.insert(subscriber);
}

void Publisher::NotifySubscribers()
{
	for (auto subscriber: m_subscribers) {
		subscriber->Notify();
	}
}

} // end_of_namespace
} // end_of_namespace
