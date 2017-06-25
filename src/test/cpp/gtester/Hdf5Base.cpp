#include "Hdf5Base.h"

const boost::property_tree::ptree Tests::Hdf5Base::getConfigTree() const {
	boost::property_tree::ptree config_tree;
	config_tree.put("run.<xmlattr>.name", "testHdf5");

	config_tree.put("run.r0", 11.0);
	config_tree.put("run.start_date", "2017-01-01");
	config_tree.put("run.num_days", 50U);
	config_tree.put("run.holidays", "holidays_none.json");
	config_tree.put("run.age_contact_matrix_file", "contact_matrix_average.xml");
	config_tree.put("run.track_index_case", 0);
	config_tree.put("run.num_threads", 4);
	config_tree.put("run.information_policy", "Global");

	config_tree.put("run.outputs.log.<xmlattr>.level", "None");
	config_tree.put("run.outputs.participants_survey.<xmlattr>.num", 10);
	config_tree.put("run.outputs.checkpointing.<xmlattr>.frequency", 1);

	config_tree.put("run.disease.seeding_rate", 0.002);
	config_tree.put("run.disease.immunity_rate", 0.8);
	config_tree.put("run.disease.config", "disease_measles.xml");

	config_tree.put("run.regions.region.<xmlattr>.name", "Belgium");
	config_tree.put("run.regions.region.rng_seed", 1U);
	config_tree.put("run.regions.region.population", "bigpop.xml");
	return config_tree;
}
