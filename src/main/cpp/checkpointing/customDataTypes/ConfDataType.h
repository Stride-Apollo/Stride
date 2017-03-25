#pragma once

struct ConfDataType {
	unsigned int checkpointing_frequency;
	unsigned long rng_seed;
	unsigned int r0;
	double seeding_rate;
	double immunity_rate;
	unsigned int num_days;
	const char* output_prefix;
	bool generate_person_file;
	unsigned int num_participants_survey;
	const char* start_date; // TODO Make this a compound datatype dateString

	const char* log_level; // TODO Should we make this a int8_t? Cuz only 3 options??
};
