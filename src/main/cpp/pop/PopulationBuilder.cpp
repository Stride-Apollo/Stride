/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Initialize populations.
 */

#include "PopulationBuilder.h"

#include "util/InstallDirs.h"
#include "util/StringUtils.h"

#include <spdlog/spdlog.h>
#include <boost/property_tree/xml_parser.hpp>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace stride::util;

shared_ptr<Population> PopulationBuilder::Build(
		const boost::property_tree::ptree& pt_config,
		const boost::property_tree::ptree& pt_disease,
		util::Random& rng) {
	//------------------------------------------------
	// Setup.
	//------------------------------------------------
	const auto pop = make_shared<Population>();
	Population& population = *pop;
	const double seeding_rate = pt_config.get<double>("run.seeding_rate");
	const double immunity_rate = pt_config.get<double>("run.immunity_rate");
	const string disease_config_file = pt_config.get<string>("run.disease_config_file");

	//------------------------------------------------
	// check input.
	//------------------------------------------------
	bool status = (seeding_rate <= 1) && (immunity_rate <= 1) && ((seeding_rate + immunity_rate) <= 1);
	if (!status) {
		throw runtime_error(string(__func__) + "> Bad input data.");
	}

	//------------------------------------------------
	// Add persons to population.
	//------------------------------------------------
	const auto file_name = pt_config.get<string>("run.population_file");;
	const auto file_path = InstallDirs::getDataDir() /= file_name;
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__)
							+ "> Population file " + file_path.string() + " not present.");
	}

	boost::filesystem::ifstream pop_file;
	pop_file.open(file_path.string());
	if (!pop_file.is_open()) {
		throw runtime_error(string(__func__)
							+ "> Error opening population file " + file_path.string());
	}

	const auto distrib_start_infectiousness = getDistribution(pt_disease, "disease.start_infectiousness");
	const auto distrib_start_symptomatic = getDistribution(pt_disease, "disease.start_symptomatic");
	const auto distrib_time_infectious = getDistribution(pt_disease, "disease.time_infectious");
	const auto distrib_time_symptomatic = getDistribution(pt_disease, "disease.time_symptomatic");

	string line;
	getline(pop_file, line); // step over file header
	unsigned int person_id = 0U;
	while (getline(pop_file, line)) {
		// Make use of stochastic disease characteristics.
		const auto start_infectiousness = sample(rng, distrib_start_infectiousness);
		const auto start_symptomatic = sample(rng, distrib_start_symptomatic);
		const auto time_infectious = sample(rng, distrib_time_infectious);
		const auto time_symptomatic = sample(rng, distrib_time_symptomatic);
		const auto values = StringUtils::split(line, ",");
		population.emplace_back(Person(person_id,
									   StringUtils::fromString<unsigned int>(values[0]),
									   StringUtils::fromString<unsigned int>(values[1]),
									   StringUtils::fromString<unsigned int>(values[2]),
									   StringUtils::fromString<unsigned int>(values[3]),
									   StringUtils::fromString<unsigned int>(values[4]),
									   StringUtils::fromString<unsigned int>(values[5]),
									   start_infectiousness, start_symptomatic, time_infectious, time_symptomatic));
		++person_id;
	}

	pop_file.close();

	//------------------------------------------------
	// Customize the population.
	//------------------------------------------------

	const unsigned int max_population_index = population.size() - 1;
	if (max_population_index <= 1U) {
		throw runtime_error(string(__func__) + "> Problem with population size.");
	}
	//------------------------------------------------
	// Set participants in social contact survey.
	//------------------------------------------------
	const string log_level = pt_config.get<string>("run.log_level", "None");
	if (log_level == "Contacts") {
		const unsigned int num_participants = pt_config.get<double>("run.num_participants_survey");

		// use a while-loop to obtain 'num_participant' unique participants (default sampling is with replacement)
		// A for loop will not do because we might draw the same person twice.
		unsigned int num_samples = 0;
		const shared_ptr<spdlog::logger> logger = spdlog::get("contact_logger");
		while (num_samples < num_participants) {
			Person& p = population[rng(max_population_index)];
			if (!p.isParticipatingInSurvey()) {
				p.participateInSurvey();
				logger->info("[PART] {} {} {}", p.getId(), p.getAge(), p.getGender());
				num_samples++;
			}
		}
	}

	//------------------------------------------------
	// Set population immunity.
	//------------------------------------------------
	unsigned int num_immune = floor(static_cast<double>(population.size()) * immunity_rate);
	while (num_immune > 0) {
		Person& p = population[rng(max_population_index)];
		if (p.getHealth().isSusceptible()) {
			p.getHealth().setImmune();
			num_immune--;
		}
	}

	//------------------------------------------------
	// Seed infected persons.
	//------------------------------------------------
	unsigned int num_infected = floor(static_cast<double> (population.size()) * seeding_rate);
	while (num_infected > 0) {
		Person& p = population[rng(max_population_index)];
		if (p.getHealth().isSusceptible()) {
			p.getHealth().startInfection();
			num_infected--;
		}
	}

	//------------------------------------------------
	// Done
	//------------------------------------------------
	return pop;
}


vector<double> PopulationBuilder::getDistribution(const boost::property_tree::ptree& pt_root, const string& xml_tag) {
	vector<double> values;
	boost::property_tree::ptree subtree = pt_root.get_child(xml_tag);
	for (const auto& tree : subtree) {
		values.push_back(tree.second.get<double>(""));
	}
	return values;
}

unsigned int PopulationBuilder::sample(Random& rng, const vector<double>& distribution) {
	double random_value = rng.nextDouble();
	for (unsigned int i = 0; i < distribution.size(); i++) {
		if (random_value <= distribution[i]) {
			return i;
		}
	}
	cerr << "WARNING: PROBLEM WITH DISEASE DISTRIBUTION [PopulationBuilder]" << endl;
	return distribution.size();
}

}
