
#pragma once
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <utility>
#include <fstream>
#include <map>

#include "sim/Simulator.h"
#include "core/Cluster.h"
#include "util/GeoCoordCalculator.h"
#include "util/Observer.h"


namespace stride {

using boost::property_tree::ptree;
using std::string;
using std::pair;
using std::ofstream;
using std::vector;
using std::map;
using std::pair;

class ClusterSaver : public util::Observer<Simulator> {
public:
	ClusterSaver(string file_name, string pop_file_name, string facility_file_name, string output_dir);

	virtual void update(const Simulator& sim) {
		saveClustersCSV(sim);
		savePopDataJSON(sim);
		saveTransportationFacilities(sim);
		m_sim_day++;
	}

private:
	using uint = unsigned int;

	/// Saves cluster information for Households (aggregated), Primary Communities and Secondary Communities.
	void saveClustersCSV(const Simulator& sim) const;

	/// Saves a single cluster.
	inline void saveClusterCSV(const Cluster& cluster, ofstream& csv_file) const;

	/// Aggregates the vector of given clusters according to their GeoLocation, and saves them.
	void saveAggrClustersCSV(const vector<Cluster>& households, ofstream& csv_file) const;

	/// Saves an aggregated cluster. The clusters that need to be aggregated are given by the indices.
	void saveClusterGroup(const vector<Cluster>& households, const vector<uint> indices, ofstream& csv_file) const;

	// Deprecated
	void saveClustersJSON(const Simulator& sim) const;
	// Deprecated
	pair<ptree, ptree> getClusterJSON(const Cluster& cluster) const;

	// Save data of the population to a JSON file
	void savePopDataJSON(const Simulator& local_sim) const;

	double getPopCount(const Simulator& local_sim) const;

	map<uint, uint> getAgeMap(const Simulator& local_sim) const;

	void saveTransportationFacilities(const Simulator& local_sim) const;


private:
	uint m_sim_day = 0;
	string m_file_name;
	string m_file_dir;
	string m_pop_file_name;
	string m_pop_file_dir;
	string m_facility_file_name;
	string m_facility_file_dir;
};

template<ClusterType type>
class ClusterCalculator {
public:
	static double calcSurface(const vector<Cluster>& clusters) {
		double latitude_middle = 0.0;
		double longitude_middle = 0.0;
		const auto& calc = GeoCoordCalculator::getInstance();

		for (const auto& cluster: clusters) {
			latitude_middle += cluster.getLocation().m_latitude;
			longitude_middle += cluster.getLocation().m_longitude;
		}

		latitude_middle /= clusters.size() - 1;
		longitude_middle /= clusters.size() - 1;

		GeoCoordinate middle;
		middle.m_latitude = latitude_middle;
		middle.m_longitude = longitude_middle;

		double radius = 0.0;

		for (auto it = clusters.begin() + 1; it != clusters.end(); ++it) {
			const auto& cluster = *it;

			double candidate_distance = calc.getDistance(middle, cluster.getLocation());

			if (radius < candidate_distance && cluster.getActiveClusterMembers() != 0) {
				radius = candidate_distance;
			}
		}

		return PI * radius * radius;
	}

	static map<uint, uint> getClMap(const vector<Cluster>& clusters) {
		map<uint, uint> result;

		for (const auto& cluster: clusters) {
			uint count = cluster.getActiveClusterMembers();

			if (result.find(count) == result.end() && count != 0) {
				result[count] = 1;
			} else {
				++result[count];
			}

		}

		return result;
	}

	/// Calculate the used surface
	/// The middle point of cities is calculated, followed by the distance between this point and the furthest city
	/// The surface is then determined by calculating the surface of the circle determined by the middle and this distance (radius)
	static double calculateSurface(const Simulator& local_sim) {
		throw runtime_error (string(__func__) + ">Trying to save unknown cluster type.");
	}

	static map<uint, uint> getClusterMap(const Simulator& local_sim) {
		throw runtime_error (string(__func__) + ">Trying to save unknown cluster type.");
	}
};

template<>
class ClusterCalculator<ClusterType::Household> {
public:
	static double calculateSurface(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.getHouseholds());
	}

	static map<uint, uint> getClusterMap(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.getHouseholds());
	}
};

template<>
class ClusterCalculator<ClusterType::School> {
public:
	static double calculateSurface(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.getSchoolClusters());
	}

	static map<uint, uint> getClusterMap(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.getSchoolClusters());
	}
};

template<>
class ClusterCalculator<ClusterType::Work> {
public:
	static double calculateSurface(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.getWorkClusters());
	}

	static map<uint, uint> getClusterMap(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.getWorkClusters());
	}
};

template<>
class ClusterCalculator<ClusterType::PrimaryCommunity> {
public:
	static double calculateSurface(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.getPrimaryCommunities());
	}

	static map<uint, uint> getClusterMap(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.getPrimaryCommunities());
	}
};

template<>
class ClusterCalculator<ClusterType::SecondaryCommunity> {
public:
	static double calculateSurface(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.getSecondaryCommunities());
	}

	static map<uint, uint> getClusterMap(const Simulator& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.getSecondaryCommunities());
	}
};


}
