
#include <iostream>
#include <string>

#include "PopulationGenerator.h"

using namespace std;
using namespace stride;
using namespace popgen;

int main(int argc, char** argv) {
	if (argc <= 1) {
		cerr << "Please provide a file as first argument.\n";
		return 1;
	}
	cerr << "Starting...\n";
	PopulationGenerator generator {"PopGenerator.xml"};
	cerr << "Generating...\n";
	generator.generate();
	cerr << "Done!\n";
}
