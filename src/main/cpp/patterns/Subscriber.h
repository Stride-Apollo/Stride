#ifndef SUBSCRIBER_H_INCLUDED
#define SUBSCRIBER_H_INCLUDED

/**
 * @file
 * Header for the Subscriber class.
 */

namespace stride {
namespace patterns {

using namespace std;

/**
 * The Subscriber in the publisher-subscriber pattern
 */
class Subscriber
{
public:
	/// Constructor
	Subscriber() {}

	/// Destructor:
	virtual ~Subscriber() {}

	/// The function called by the publisher to notify its subscribers
	virtual void Notify() = 0;

};

} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
