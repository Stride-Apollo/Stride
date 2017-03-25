#pragma once

struct ConfDataType {
	unsigned int checkpointing_frequency;
	unsigned long rng_seed;
	unsigned int r0;
	double seeding_rate;
	double immunity_rate;
	unsigned int num_days;
	char* output_prefix; // TODO In our format we have a compound datatype of a string.
	bool generate_person_file;
	unsigned int num_participants_survey;
	char* start_date; // TODO Make this a compound datatype dateString
	int8_t log_level;
};
