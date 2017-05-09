
#include <iostream>
#include <string>
#include <random>
#include <tclap/CmdLine.h>

#include "PopulationGenerator.cpp"

using namespace std;
using namespace stride;
using namespace popgen;
using namespace TCLAP;

int main(int argc, char** argv) {
	try{
		// TCLAP commandline interface
		CmdLine cmd("Commandline interface of the PopulationGenerator", ' ', "Beta");
		
		// The source xml file argument
		ValueArg<string> sourceArg("s","source","Source xml file for the configuration of the generator",true,"data/happy_day.xml","string");
		cmd.add(sourceArg);

		// The target city file argument
		ValueArg<string> targetCitiesArg("c","targetcity","The target cities file",true,"generatedCities","string");
		cmd.add(targetCitiesArg);

		// The target city file argument
		ValueArg<string> targetPopulationArg("p","targetpopulation","The target population file",true,"generatedPopulation","string");
		cmd.add(targetPopulationArg);

		// The target household file argument
		// Use flag d instead of h (standard help flag)
		// d stands for domiciliary (which is a synonym for household)
		ValueArg<string> targetHouseholdArg("d","targethousehold","The target households file",true,"generatedHouseholds","string");
		cmd.add(targetHouseholdArg);

		// The target cluster file argument
		// Flag c is already taken so take flag g (group) instead
		ValueArg<string> targetClustersArg("g","targetcluster","The target clusters file",true,"generatedClusters","string");
		cmd.add(targetClustersArg);

		// Parse the argv array
		cmd.parse(argc, argv);

		// Get the value parsed by each argument
		string sourceXml = sourceArg.getValue();
		string targetCities = targetCitiesArg.getValue();
		string targetPopulation = targetPopulationArg.getValue();
		string targetHousehold = targetHouseholdArg.getValue();
		string targetClusters = targetClustersArg.getValue();

		cerr << "Starting...\n";
		// TODO support multiple random generators here
		PopulationGenerator<std::mt19937> generator {sourceXml};
		cerr << "Generating...\n";
		generator.generate(targetCities, targetPopulation, targetHousehold, targetClusters);
		cerr << "Done!\n";
	} catch (ArgException &exc)
	{ cerr << "error: " << exc.error() << " for arg " << exc.argId() << endl; }
}
