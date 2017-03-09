#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "AliasDistribution.h"

namespace stride {
namespace util {

using namespace std;
using boost::property_tree::ptree;

class PopGenException;

struct SimpleCluster {
	unsigned int maxSize;
	unsigned int currentSize;
	unsigned int id;
	string name;
};

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
	PopulationGenerator(const string& fileName);
	/// TODO add throw?

	/**
	 * Generate a new population and write it to a file.
	 *
	 * @param targetFile		The name of the file in which you want to store the generated population.
	 */
	void makePopulation(const string& targetFile) const;

private:
	template<typename RNG>
	unsigned int makeFamily(unsigned int maxSize, ofstream& file, RNG& rng) const;

	template<typename RNG>
	void makeFamilyNoChildren(unsigned int size, ofstream& file, RNG& rng) const;

	template<typename RNG>
	void makeFamilyWithChildren(unsigned int size, ofstream& file, RNG& rng) const;

	template<typename RNG>
	unsigned int makeFamilySize(unsigned int maxSize, RNG& rng) const;

	boost::property_tree::ptree m_propTree;
};

template<typename RNG>
unsigned int PopulationGenerator::makeFamily(unsigned int maxSize, ofstream& file, RNG& rng) const {
	if (maxSize == 0) {
		return 0;
	}

	unsigned int familySize = makeFamilySize(maxSize, rng);

	if (familySize < m_propTree.get<unsigned int>("POPULATION.FAMILY.NOCHILDREN.<xmlattr>.min")
			|| familySize > m_propTree.get<unsigned int>("POPULATION.FAMILY.NOCHILDREN.<xmlattr>.min")) {

		makeFamilyNoChildren(familySize, file, rng);
	} else {
		makeFamilyWithChildren(familySize, file, rng);
	}

	return familySize;
}

template<typename RNG>
unsigned int stride::util::PopulationGenerator::makeFamilySize(unsigned int maxSize, RNG& rng) const {
	vector<double> fractions;
	vector<unsigned int> sizes;

	for (ptree::const_iterator it = m_propTree.get_child("POPULATION.FAMILY.FAMILYSIZE").begin();
			it != m_propTree.get_child("POPULATION.FAMILY.FAMILYSIZE").end(); it++) {

		if (it->first != "<xmlcomment>" && it->second.get<unsigned int>("<xmlattr>.size") <= maxSize) {
			fractions.push_back(it->second.get<unsigned int>("<xmlattr>.fraction") / 100.0);
			sizes.push_back(it->second.get<unsigned int>("<xmlattr>.size"));
		}
	}

	AliasDistribution dist (fractions);
	return sizes.at(dist(rng));
}

template<typename RNG>
void PopulationGenerator::makeFamilyNoChildren(unsigned int size, ofstream& file, RNG& rng) const {
	/// NOTE: I assume the family members have to be over 18


}

template<typename RNG>
void PopulationGenerator::makeFamilyWithChildren(unsigned int size, ofstream& file, RNG& rng) const {
}

}
}
