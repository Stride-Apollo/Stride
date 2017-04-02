#pragma once

struct ConfDataType {
	unsigned int checkpointing_frequency;
	unsigned long rng_seed;
	const char* disease_config_file;
	const char* holidays_file;
	const char* age_contact_matrix_file;
	const char* checkpointing_file;
	unsigned int r0;
	double seeding_rate;
	double immunity_rate;
	unsigned int num_days;
	const char* output_prefix;
	bool generate_person_file;
	unsigned int num_participants_survey;
	const char* start_date;
	const char* log_level;
	const char* population_file;
};
