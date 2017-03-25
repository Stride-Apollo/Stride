#include "FamilyParser.h"
#include <iostream>

using namespace stride;
using namespace popgen;
using namespace std;

vector<FamilyConfig> FamilyParser::parseFamilies(string filename) {
	vector<FamilyConfig> result;

	string line;
	ifstream myfile (filename);
	if (myfile.is_open()) {
		while (getline(myfile,line)) {
			if (line != "") {
				result.push_back(parseFamily(line));
			}
		}
		myfile.close();
	} else {
		/// TODO exception
		cout << "Unable to open file\n";
	}

	return result;
}

FamilyConfig FamilyParser::parseFamily(string config) {
	FamilyConfig result;

	for (char& c: config) {
		static uint current_age = 0;

		if (c == ' ') {
			result.push_back(current_age);
			current_age = 0;
		} else if(&c == &config.back()) {
			current_age *= 10;
			current_age += uint(c) - uint('0');
			result.push_back(current_age);
			current_age = 0;
		} else if (c >= int('0') && c <= int('9')) {
			current_age *= 10;
			current_age += uint(c) - uint('0');
		} else {
			/// TODO throw exception
		}
	}

	return result;
}