#pragma once

/**
 * @file
 * Header for the Publisher class.
 */

#include <set>

namespace stride {
namespace patterns {

using namespace std;

class Subscriber;
/**
 * The Publisher in the publisher-Publisher pattern
 */
class Publisher
{
public:
	/// Constructor
	Publisher() {}

	/// Destructor:
	~Publisher() {}

	/// The function called by the subscriber to listen to the events of this publisher
	void Listen(Subscriber* subscriber);

private:
	/// Used to notify all subscribers an event has occurred
	void NotifySubscribers();

	set<Subscriber*> m_subscribers;

};

} // end_of_namespace
} // end_of_namespace

