
#include <iostream>
#include <string>

#include "PopulationGenerator.h"

using namespace std;
using namespace stride;
using namespace popgen;

int main(int argc, char** argv) {
	if (argc <= 1) {
		cerr << "Please provide an XML file as first argument.\n";
		return 1;
	}
	cerr << "Parsing...\n";
	PopulationGenerator popgen {string(argv[1])};
	cerr << "Starting...\n";
	Population pop = popgen.generate();
	cout << pop;
	cerr << "Done!\n";
	return 0;
}
