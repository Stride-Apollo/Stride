/*
 * PopulationGenerator.cpp
 *
 *  Created on: Mar 9, 2017
 *      Author: sam
 */

#include "PopulationGenerator.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>


#include <iostream>
#include <random>
#include <map>

using namespace stride;
using namespace util;

using namespace std;
using boost::property_tree::ptree;

PopulationGenerator::PopulationGenerator(const string& fileName) {
	try {
		read_xml(fileName, m_propTree);
		this->makePopulation("test.csv");
	} catch (const boost::property_tree::xml_parser::xml_parser_error& ex) {
		/// Let someone else deal with it, it's not my responsibility
		throw ex;
	}
}

void PopulationGenerator::makePopulation(const string& targetFile) const {

	/// TODO choose between the rngs, set the seed, catch exceptions
	random_device rng;

	ofstream file;
	file.open(targetFile);
	file << "\"age\",\"household_id\",\"school_id\",\"work_id\",\"primary_community\",\"secondary_community\"\n";

	/// The size of the community
	unsigned int currentSize = 0;
	unsigned int maximumSize = m_propTree.get<unsigned int>("POPULATION.COMMUNITY.<xmlattr>.size");

//	while (currentSize != maximumSize){
//		unsigned int newMembers = makeFamily(maximumSize - currentSize, file);
//		currentSize += newMembers;
//	}
	unsigned int newSize = makeFamilySize(10, rng);

	cout << "I was able to make a family of size " << newSize << endl;

	file.close();
}

int main() {
	PopulationGenerator test = PopulationGenerator("./test.xml");
}

