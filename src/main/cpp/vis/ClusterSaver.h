
#pragma once
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <utility>
#include <fstream>
#include <map>

#include "sim/LocalSimulatorAdapter.h"
#include "core/Cluster.h"
#include "util/GeoCoordCalculator.h"
#include "util/Observer.h"

using boost::property_tree::ptree;
using std::string;
using std::pair;
using std::ofstream;
using std::vector;
using std::map;


namespace stride {

class ClusterSaver : public util::Observer<LocalSimulatorAdapter> {
public:
	ClusterSaver(string file_name, string pop_file_name);

	virtual void update(const LocalSimulatorAdapter& sim) {
		saveClustersCSV(sim);
		savePopDataJSON(sim);
		m_sim_day++;
	}

private:
	using uint = unsigned int;

	/// Saves cluster information for Households (aggregated), Primary Communities and Secondary Communities.
	void saveClustersCSV(const LocalSimulatorAdapter& local_sim) const;

	/// Saves a single cluster.
	inline void saveClusterCSV(const Cluster& cluster, ofstream& csv_file) const;

	/// Aggregates the vector of given clusters according to their GeoLocation, and saves them.
	void saveAggrClustersCSV(const vector<Cluster>& households, ofstream& csv_file) const;

	/// Saves an aggregated cluster. The clusters that need to be aggregated are given by the indices.
	void saveClusterGroup(const vector<Cluster>& households, const vector<uint> indices, ofstream& csv_file) const;

	// Deprecated
	void saveClustersJSON(const LocalSimulatorAdapter& local_sim) const;
	// Deprecated
	pair<ptree, ptree> getClusterJSON(const Cluster& cluster) const;

	// Save data of the population to a JSON file
	void savePopDataJSON(const LocalSimulatorAdapter& local_sim) const;

	double getPopCount(const LocalSimulatorAdapter& local_sim) const;

	map<uint, uint> getAgeMap(const LocalSimulatorAdapter& local_sim) const;


private:
	uint m_sim_day = 0;
	string m_file_name;
	string m_file_dir;
	string m_pop_file_name;
	string m_pop_file_dir;
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

		latitude_middle /= clusters.size();
		longitude_middle /= clusters.size();

		GeoCoordinate middle;
		middle.m_latitude = latitude_middle;
		middle.m_longitude = longitude_middle;

		double radius = 0.0;
		for (const auto& cluster: clusters) {
			double candidate_distance = calc.getDistance(middle, cluster.getLocation());

			if (radius < candidate_distance) {
				radius = candidate_distance;
			}
		}

		return PI * radius * radius;
	}

	static map<uint, uint> getClMap(const vector<Cluster>& clusters) {
		map<uint, uint> result;

		for (const auto& cluster: clusters) {
			if (cluster.getSize() > 30) {
				cout << "FOUND CL " << toString(cluster.getClusterType()) << " " << cluster.getSize() << " ID " << cluster.getId() << endl;
				cin.ignore();
			}
			cout << "HANDLED CL " << toString(cluster.getClusterType()) << " " << cluster.getSize() << " ID " << cluster.getId() << endl;

			if (result.find(cluster.getSize()) == result.end() && cluster.getSize() != 0) {
				result[cluster.getSize()] = 0;
			} else {
				++result[cluster.getSize()];
			}

		}

		return result;
	}

	/// Calculate the used surface
	/// The middle point of cities is calculated, followed by the distance between this point and the furthest city
	/// The surface is then determined by calculating the surface of the circle determined by the middle and this distance (radius)
	static double calculateSurface(const LocalSimulatorAdapter& local_sim) {
		throw runtime_error (string(__func__) + ">Trying to save unknown cluster type.");
	}

	static map<uint, uint> getClusterMap(const LocalSimulatorAdapter& local_sim) {
		throw runtime_error (string(__func__) + ">Trying to save unknown cluster type.");
	}
};

template<>
class ClusterCalculator<ClusterType::Household> {
public:
	static double calculateSurface(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.m_sim->m_households);
	}

	static map<uint, uint> getClusterMap(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.m_sim->m_households);
	}
};

template<>
class ClusterCalculator<ClusterType::School> {
public:
	static double calculateSurface(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.m_sim->m_school_clusters);
	}

	static map<uint, uint> getClusterMap(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.m_sim->m_school_clusters);
	}
};

template<>
class ClusterCalculator<ClusterType::Work> {
public:
	static double calculateSurface(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.m_sim->m_work_clusters);
	}

	static map<uint, uint> getClusterMap(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.m_sim->m_work_clusters);
	}
};

template<>
class ClusterCalculator<ClusterType::PrimaryCommunity> {
public:
	static double calculateSurface(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.m_sim->m_primary_community);
	}

	static map<uint, uint> getClusterMap(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.m_sim->m_primary_community);
	}
};

template<>
class ClusterCalculator<ClusterType::SecondaryCommunity> {
public:
	static double calculateSurface(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::calcSurface(local_sim.m_sim->m_secondary_community);
	}

	static map<uint, uint> getClusterMap(const LocalSimulatorAdapter& local_sim) {
		return ClusterCalculator<ClusterType::Null>::getClMap(local_sim.m_sim->m_secondary_community);
	}
};


}
