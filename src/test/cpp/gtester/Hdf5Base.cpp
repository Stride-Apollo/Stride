#include "Hdf5Base.h"

const boost::property_tree::ptree Tests::Hdf5Base::getConfigTree() const {
	boost::property_tree::ptree config_tree;
	config_tree.put("run.rng_seed", 1U);
	config_tree.put("run.r0", 11.0);
	config_tree.put("run.seeding_rate", 0.002);
	config_tree.put("run.immunity_rate", 0.8);
	config_tree.put("run.population_file", "small_pop.csv");
	config_tree.put("run.num_days", 50U);
	config_tree.put("run.output_prefix", "testHdf5");
	config_tree.put("run.disease_config_file", "disease_measles.xml");
	config_tree.put("run.num_participants_survey", 10);
	config_tree.put("run.start_date", "2017-01-01");
	config_tree.put("run.holidays_file", "holidays_none.json");
	config_tree.put("run.age_contact_matrix_file", "contact_matrix_average.xml");
	config_tree.put("run.log_level", "None");
	config_tree.put("run.generate_person_file", 0);
	config_tree.put("run.checkpointing_file", "testOutput.h5");
	config_tree.put("run.checkpointing_frequency", 1);
	return config_tree;
}
