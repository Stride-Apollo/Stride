#pragma once

#include <future>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#include "sim/Simulator.h"
#include "util/RNGPicker.h"
#include "util/GeoCoordinate.h"
#include "core/Cluster.h"

namespace stride {

using namespace std;
using namespace util;

class AsyncSimulator {
public:
	/// The bool doesn't matter, C++ can't handle void
	/// We just need to wait until it is done
	virtual future<bool> timeStep() = 0;

	/// Receive travelers
	virtual bool host(const vector<Simulator::TravellerType>& travellers, uint days, string destination_district, string destination_facility) = 0;

	/// Return these travellers back home (in this simulator instance)
	virtual bool returnHome(const vector<Simulator::TravellerType>& travellers) = 0;

	/// Send travellers to the destination region
	/// Returns a vector of indices (in the Population of the simulator), these indices are from the people that were sent (debugging purposes)
	virtual vector<unsigned int> sendTravellers(uint amount, uint days, AsyncSimulator* destination_sim, string destination_district, string destination_facility) = 0;

	/// Return an index to a cluster in the given vector
	/// Current policy: search for the first cluster with equal coordinates
	/// Return the size of the vectpr if you can't find any
	uint chooseCluster(const GeoCoordinate& coordinate, const vector<Cluster>& clusters) {
		for (uint i = 0; i < clusters.size(); ++i) {
			if (i == 0) {
				// Because 0 is an invalid id and thus it must be skipped
				continue;
			}

			const auto& cluster = clusters.at(i);

			if (coordinate == cluster.getLocation()) {
				return i;
			}
		}
		return clusters.size();
	}

	virtual ~AsyncSimulator() {};

	AsyncSimulator(string rng_name = "MT19937", uint seed = rand()) {m_rng.set(rng_name, seed);}

protected:
	RNGPicker m_rng;
};

}
