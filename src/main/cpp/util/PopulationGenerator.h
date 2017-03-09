#pragma once

#include <string>
#include <vector>

namespace stride {
namespace util {

using namespace std;

class PopGenException;

/// Usage:
/// Construct the object using an xml file which contains the configuration of the population
/// write a newly generated population to a file using the AliasDistribution::makePopulation method
class PopulationGenerator {
public:
	/**
	 * Make a generator that generates populations based on a configuration file.
	 *
	 * @param fileName		A string indicating the name of the configuration file.
	 */
	PopulationGenerator(const string& fileName) throw(PopGenException);

	/**
	 * Generate a new population and write it to a file.
	 *
	 * @param targetFile		The name of the file in which you want to store the generated population.
	 */
	void makePopulation(const string& targetFile) const;

private:
	void parsePopulation();
};

}
}
