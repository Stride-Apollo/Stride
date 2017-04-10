
#include <iostream>
#include <string>

#include "PopulationGenerator.h"

using namespace std;
using namespace stride;
using namespace popgen;

int main(int argc, char** argv) {
	if (argc < 5) {
		cerr << "Please provide a file as first argument.\n";
		return 1;
	}
	cerr << "Starting...\n";
	PopulationGenerator generator {argv[1]};
	cerr << "Generating...\n";
	generator.generate(argv[2], argv[3], argv[4]);
	cerr << "Done!\n";
}
