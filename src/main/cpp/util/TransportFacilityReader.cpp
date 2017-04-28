#include "TransportFacilityReader.h"
#include "StringUtils.h"

#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace stride;
using namespace util;

vector<pair<string, string> > TransportFacilityReader::readFacilities(string filename) {
	ifstream my_file(filename.c_str());
	if (!my_file.good()) {
		throw invalid_argument("Invalid facility file.");
	}

	vector<pair<string, string> > result;

	string line;
	while (getline(my_file,line)) {
		result.push_back(parseFacility(line));
	}

	return result;
}

pair<string, string> parseFacility(string row) {
	vector<string> values = StringUtils::split(row, ",");
	
	if (values.size() != 2) {
		throw invalid_argument("Facility file has rows containing " + StringUtils::toString(values.size()) + " elements, it should be 2.");
	}

	return make_pair(values[0], values[1]);
}