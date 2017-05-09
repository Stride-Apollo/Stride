
#include <iostream>
#include <string>
#include <random>
#include <tclap/CmdLine.h>

#include "PopulationGenerator.cpp"

using namespace std;
using namespace stride;
using namespace popgen;
using namespace TCLAP;

template<class T>
void run(T generator, const string targetCities, const string targetPopulation, const string targetHousehold, const string targetClusters){
	cerr << "Generating...\n";
	generator.generate(targetCities, targetPopulation, targetHousehold, targetClusters);
	cerr << "Done!\n";
}

int main(int argc, char** argv) {
	try{
		// TCLAP commandline interface
		CmdLine cmd("Commandline interface of the PopulationGenerator", ' ', "Beta");

		// The source xml file argument
		ValueArg<string> sourceArg("s", "source", "Source xml file for the configuration of the generator", true, "data/happy_day.xml", "string");
		cmd.add(sourceArg);

		// The target city file argument
		ValueArg<string> targetCitiesArg("c", "targetcity", "The target cities file", true, "generatedCities", "string");
		cmd.add(targetCitiesArg);

		// The target city file argument
		ValueArg<string> targetPopulationArg("p", "targetpopulation", "The target population file", true, "generatedPopulation", "string");
		cmd.add(targetPopulationArg);

		// The target household file argument
		// Use flag d instead of h (standard help flag)
		// d stands for domiciliary (which is a synonym for household)
		ValueArg<string> targetHouseholdArg("d", "targethousehold", "The target households file", true, "generatedHouseholds", "string");
		cmd.add(targetHouseholdArg);

		// The target cluster file argument
		// Flag c is already taken so take flag g (group) instead
		ValueArg<string> targetClustersArg("g", "targetcluster", "The target clusters file", true, "generatedClusters", "string");
		cmd.add(targetClustersArg);

		// The random generator argument
		string options = "The random generator(one of the following): ";
 		options += "default_random_engine - mt19937 - mt19937_64 - minstd_rand0 - minstd_rand - ranlux24_base - ranlux48_base - ranlux24 - ranlux48 - knuth_b";
		ValueArg<string> rngArg("r", "randomgenerator", options, false, "mt19937", "string");
		cmd.add(rngArg);

		// Parse the argv array
		cmd.parse(argc, argv);

		// Get the value parsed by each argument
		string sourceXml = sourceArg.getValue();
		string targetCities = targetCitiesArg.getValue();
		string targetPopulation = targetPopulationArg.getValue();
		string targetHousehold = targetHouseholdArg.getValue();
		string targetClusters = targetClustersArg.getValue();
		string rng = rngArg.getValue();

		cerr << "Starting...\n";
		if (rng == "default_random_engine"){
			PopulationGenerator<default_random_engine> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "mt19937"){
			PopulationGenerator<mt19937> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "mt19937_64"){
			PopulationGenerator<mt19937_64> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "minstd_rand0"){
			PopulationGenerator<minstd_rand0> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "minstd_rand"){
			PopulationGenerator<minstd_rand> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "ranlux24_base"){
			PopulationGenerator<ranlux24_base> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "ranlux48_base"){
			PopulationGenerator<ranlux48_base> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "ranlux24"){
			PopulationGenerator<ranlux24> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "ranlux48"){
			PopulationGenerator<ranlux48> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
		else if (rng == "knuth_b"){
			PopulationGenerator<knuth_b> generator {sourceXml};
			run(generator, targetCities, targetPopulation, targetHousehold, targetClusters);
		}
	} catch (ArgException &exc){
		cerr << "error: " << exc.error() << " for arg " << exc.argId() << endl;
	}
}
