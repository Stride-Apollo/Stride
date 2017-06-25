#include "FamilyParser.h"
#include <iostream>
#include <stdexcept>
#include "util/InstallDirs.h"

using namespace stride;
using namespace popgen;
using namespace std;
using namespace util;

vector<FamilyConfig> FamilyParser::parseFamilies(string filename) const {
	// Precond: datdir OK
	vector<FamilyConfig> result;

	string line;
	ifstream myfile((InstallDirs::getDataDir() /= filename).string());
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			if (line != "") {
				result.push_back(parseFamily(line));
			}
		}
		myfile.close();
	} else {
		throw invalid_argument("In FamilyParser: Invalid file name");
	}

	return result;
}

FamilyConfig FamilyParser::parseFamily(string config) const {
	FamilyConfig result;

	for (char& c: config) {
		static uint current_age = 0;
		static bool encountered_space = false;

		if (c == ' ') {
			if (not encountered_space) {
				result.push_back(current_age);
			}
			encountered_space = true;
			current_age = 0;
		} else if (&c == &config.back()) {
			encountered_space = false;
			current_age *= 10;
			current_age += uint(c) - uint('0');
			result.push_back(current_age);
			current_age = 0;
		} else if (c >= int('0') && c <= int('9')) {
			encountered_space = false;
			current_age *= 10;
			current_age += uint(c) - uint('0');
		} else {
			throw invalid_argument("In FamilyParser: Encountered something that isn't an unsigned integer or a space");
		}
	}

	return result;
}