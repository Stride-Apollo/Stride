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
#include "pop/TravellerData.h"

namespace stride {

using namespace std;
using namespace util;

class AsyncSimulator {
public:
	void setId(uint id) {m_id = id;}
	
	uint getId() const {return m_id;}

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
		// TODO extend with sphere of influence
		vector<uint> available_clusters;
		for (uint i = 1; i < clusters.size(); ++i) {

			const auto& cluster = clusters.at(i);

			if (coordinate == cluster.getLocation()) {
				available_clusters.push_back(i);
			}
		}

		if (available_clusters.size() != 0) {
			uniform_int_distribution<int> dist(0,available_clusters.size() - 1);
			return available_clusters[dist(m_rng)];

		} else
			return clusters.size();
	}

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Receive a traveller, get your new IDs of the clusters from the traveller data
	virtual bool forceHost(const Simulator::TravellerType& traveller, const TravellerData& traveller_data) = 0;

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Return all travellers in this simulator back home
	virtual vector<TravellerData> forceReturn() = 0;

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Does the same as AsyncSimulator::forceReturn except for the fact that the travellers aren't sent back home
	virtual vector<TravellerData> getTravellerData() = 0;

	/// Helps to integrate multi region and HDF5 checkpointing
	/// Force send a traveller to a simulator
	virtual void forceSend(const TravellerData& traveller_data, AsyncSimulator* destination_sim) = 0;

	virtual ~AsyncSimulator() {};

	AsyncSimulator(string rng_name = "MT19937", uint seed = rand()) {m_rng.set(rng_name, seed);}

protected:
	RNGPicker m_rng;
	uint m_id = 0;
};

}
