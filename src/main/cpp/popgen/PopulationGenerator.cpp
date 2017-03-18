#include "PopulationGenerator.h"

#include "AgeDistribution.h"

#include <boost/property_tree/xml_parser.hpp>

using namespace stride;
using namespace popgen;
using namespace util;
using namespace boost::property_tree;
using namespace xml_parser;

std::ostream& popgen::operator<<(std::ostream& os, const Population& p) {
	os << "\"age\",\"household_id\",\"school_id\",\"work_id\",\"primary_community\",\"secondary_community\"\n";
	for (const SimplePerson& person: p.all) {
		os << person;
	}
	return os;
}

PopulationGenerator::PopulationGenerator(const string& filename) {
	read_xml(filename, m_props, trim_whitespace | no_comments);
	m_total = m_props.get<uint>("POPULATION.<xmlattr>.total");

	getFamilySizes();

	getFamilyComposition();

	makeRNG();
}

Population PopulationGenerator::generate() {
	Population pop;
	AgeDistribution age_dist(
			m_total,
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.min"),
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.max"),
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.constantUpTo")
	);

	map<uint, uint> age_map;
	uint family_id = 1;
	for (uint i=age_dist.getMin(); i<=age_dist.getMax(); i++) {
		age_map[i] = 0;
	}

	makeFamiliesWithChildren(age_map, pop, age_dist, family_id);

	makeChildlessFamilies(age_map, pop, age_dist, family_id);

	m_cluster_id = 1;

	/// schools
	makeSchools(age_map, pop);

	/// work
	makeWork(age_map, pop);

	/// communities
	makeCommunities(age_map, pop);

	return pop;
}

void PopulationGenerator::makeRNG() {
	auto rng_config = m_props.get_child("POPULATION.RANDOM");
	uint seed = rng_config.get<uint>("<xmlattr>.seed");
	string generator_type = rng_config.get<string>("<xmlattr>.generator");

	m_rng.set(generator_type, seed);
}

void PopulationGenerator::getFamilySizes() {
	double sum = 0.0;
	for (auto& size_tree: m_props.get_child("POPULATION.FAMILY.FAMILYSIZE")) {
		double frac = size_tree.second.get<double>("<xmlattr>.fraction") / 100.0;
		uint size = size_tree.second.get<uint>("<xmlattr>.size");
		m_family_size_fractions[size] = frac;
		sum += frac;
		if (size < m_family_size.min) m_family_size.min = size;
		if (size > m_family_size.max) m_family_size.max = size;
	}
	if (abs(sum-1.0) > 0.0001) {
		throw runtime_error("Sum of family size fractions does not equal 100"); // TODO better exception type
	}
}

void PopulationGenerator::getFamilyComposition() {
	map<uint, double> family_size_fractions_no_kids;
	map<uint, double> family_size_fractions_some_kids;
	uint no_children_min = m_props.get<uint>("POPULATION.FAMILY.NOCHILDREN.<xmlattr>.min");
	uint no_children_max = m_props.get<uint>("POPULATION.FAMILY.NOCHILDREN.<xmlattr>.max");
	for (uint i=m_family_size.min; i<=m_family_size.max; i++) {
		// TODO check i in m_family_size_fractions
		if (no_children_min <= i and i <= no_children_max) {
			family_size_fractions_no_kids[i] = m_family_size_fractions[i];
			m_no_kids_family_size_avg += i * m_family_size_fractions[i];
		} else {
			family_size_fractions_some_kids[i] = m_family_size_fractions[i];
		}
		m_family_size_avg += i * m_family_size_fractions[i];
	}

	m_no_kids_family_size_dist = MappedAliasDistribution(family_size_fractions_no_kids);
	m_some_kids_family_size_dist = MappedAliasDistribution(family_size_fractions_some_kids);

	m_age_no_kids_min = m_props.get<uint>("POPULATION.FAMILY.NOCHILDREN.<xmlattr>.minAge");

	auto children = m_props.get_child("POPULATION.FAMILY.CHILDREN");
	m_age_kids.min = children.get<uint>("CHILDAGE.<xmlattr>.min");
	m_age_kids.max = children.get<uint>("CHILDAGE.<xmlattr>.max");
	m_age_parents.min = children.get<uint>("PARENTAGE.<xmlattr>.min");
	m_age_parents.max = children.get<uint>("PARENTAGE.<xmlattr>.max");
	m_age_diff_kids.min = children.get<uint>("AGEDIFF.CHILDREN.<xmlattr>.min");
	m_age_diff_kids.max = children.get<uint>("AGEDIFF.CHILDREN.<xmlattr>.max");
	m_age_diff_parents_max = children.get<uint>("AGEDIFF.PARENTS.<xmlattr>.max");
	m_age_diff_parents_kids_min = children.get<uint>("AGEDIFF.PARENTSCHILDREN.<xmlattr>.min");

	if (m_age_diff_parents_kids_min > m_age_parents.min) {
		// TODO better error
		throw runtime_error("Parents have to be older than the minimum difference between children and parents");
	}
}

void PopulationGenerator::makeFamiliesWithChildren(
		map<uint, uint>& age_map,
		Population& pop,
		AgeDistribution& age_dist,
		uint& family_id) {

	auto add_person = [&](const SimplePerson& p, bool newFamily = false) {
		pop.all.push_back(p);
		age_map[p.m_age]++;

		if (newFamily) {
			pop.families.push_back(SimpleFamily());
			pop.families.back().push_back(pop.all.size() - 1);
		} else {
			pop.families.back().push_back(pop.all.size() - 1);
		}
	};

	uint total_in_fam_kids = 0;
	uint max_in_fam_kids = (uint)(
			(1.0-(m_no_kids_family_size_avg/m_family_size_avg))*double(m_total)
			- (m_family_size_avg-m_no_kids_family_size_avg)/2.0 + 0.5);
	while (total_in_fam_kids <= max_in_fam_kids) {
		uint size = m_some_kids_family_size_dist(m_rng);
		assert(size >= 3);
		uint num_children = size - 2;

		uint parent1 = age_dist.get_dist(m_age_parents)(m_rng);
		uint parent2 = age_dist.get_dist(max(m_age_parents.min, parent1 - m_age_diff_parents_max),
				min(m_age_parents.max, parent1 + m_age_diff_parents_max))(m_rng);

		uint parent_min_age = min(parent1, parent2);
		uint max_age = min(parent_min_age-m_age_diff_parents_kids_min, m_age_kids.max);

		// Check for highest possible amount of children in given range
		uint possible = double(min(max_age-m_age_kids.min, m_age_diff_kids.max))/m_age_diff_kids.min;
		if (num_children > possible) continue;

		vector<uint> children_ages(num_children);
		auto dist = age_dist.get_dist(m_age_kids.min, max_age);
		while (true) {
			for (uint i=0; i<num_children; i++) {
				children_ages[i] = dist(m_rng);
			}

			sort(children_ages.begin(), children_ages.end());

			if (children_ages.back() - children_ages.front() > m_age_diff_kids.max) continue;
			bool continue_while = false;
			for (uint i=0; i<num_children-1; i++) {
				if (children_ages[i+1] - children_ages[i] < m_age_diff_kids.min) {
					continue_while = true;
					break;
				}
			}
			if (continue_while) continue;

			// If we reach this place, everything is good!
			break;
		}

		add_person(SimplePerson(parent1, family_id), true);
		add_person(SimplePerson(parent2, family_id));
		for (uint& a: children_ages) add_person(SimplePerson(a, family_id));
		total_in_fam_kids += size;
		family_id++;
		age_dist.correct(pop.all.size(), age_map);
	}
}

void PopulationGenerator::makeChildlessFamilies(
		map<uint, uint>& age_map,
		Population& pop,
		AgeDistribution& age_dist,
		uint& family_id) {

	// Step 1.b Families without kids
	auto add_person = [&](const SimplePerson& p, bool newFamily = false) {
		pop.all.push_back(p);
		age_map[p.m_age]++;

		if (newFamily) {
			pop.families.push_back(SimpleFamily());
			pop.families.back().push_back(pop.all.size() - 1);
		} else {
			pop.families.back().push_back(pop.all.size() - 1);
		}
	};

	while (pop.all.size() <= m_total-(m_no_kids_family_size_avg/2.0)) {
		uint size = m_no_kids_family_size_dist(m_rng);
		vector<uint> ages(size);
		MinMax limits(m_age_no_kids_min, age_dist.getMax());
		for (uint i=0; i<size; i++) {
			ages[i] = age_dist.get_dist(limits)(m_rng);
			limits.min = max<int>(limits.min, ages[i] - m_age_diff_parents_max);
			limits.max = min<int>(limits.max, ages[i] + m_age_diff_parents_max);
		}

		for (uint& a: ages) add_person(SimplePerson(a, family_id));
		family_id++;
		age_dist.correct(pop.all.size(), age_map);
	}
}

void PopulationGenerator::makeSchools(const map<uint, uint>& age_map, Population& pop) {
	auto schools_config = m_props.get_child("POPULATION.EDUCATION");
	uint max_school_size = schools_config.get<uint>("<xmlattr>.size");
	vector<double> school_fractions;
	vector<MinMax> school_ages;
	vector<uint> total_schools_needed;

	for (auto it = schools_config.begin(); it != schools_config.end(); it++) {
		if (it->first == "INSTITUTION") {
			/// Get the ages for this school level
			uint min = it->second.get<uint>("<xmlattr>.minAge");
			uint max = it->second.get<uint>("<xmlattr>.maxAge");
			school_ages.push_back(MinMax(min, max));

			/// Get the fraction of people (of the orrect age) that go to this type of school
			school_fractions.push_back(it->second.get<double>("<xmlattr>.fraction") / 100.0);

			/// Determine the amount of schools needed
			/// TODO: how do i make this consistent with the fractions per school?
			uint required_school_capacity = 0;
			for (uint i = min; i <= max; i++) {
				required_school_capacity += age_map.at(i);
			}
			uint schools_needed = uint(required_school_capacity / max_school_size + 0.5);
			total_schools_needed.push_back(schools_needed);
		}
	}

	/// Make the schools
	vector<vector<SimpleSchool> > schools;
	for (uint i = 0; i < total_schools_needed.size(); i++) {
		schools.push_back(vector<SimpleSchool>());

		int places_still_needed = int (total_schools_needed[i]) * int(max_school_size);

		while (places_still_needed > 0) {
			SimpleSchool new_school;
			new_school.m_current_pupils = 0;
			new_school.m_id = m_cluster_id;
			m_cluster_id++;

			schools[i].push_back(new_school);
			places_still_needed -= max_school_size;
		}
	}

	/// Go trough the list of people and add them to an appropriate school
	/// NOTE: if one has been assigned to a schook, he can't be assigned to another school
	/// NOTE2: school age ranges might overlap but that's fine (e.g. due to specialization years)
	for (SimplePerson& person: pop.all) {
		for (uint i = 0U; i < school_ages.size(); i++) {
			MinMax& range = school_ages[i];
			if (person.m_age >= range.min and person.m_age <= range.max) {
				double fraction = school_fractions[i];
				AliasDistribution dist = AliasDistribution({fraction, 1.0 - fraction});

				if (dist(m_rng) == 0) {
					/// The person should be assigned to a school, now randomly pick one
					uint chosen_school = m_rng() % schools[i].size();

					person.m_school_id = schools[i][chosen_school].m_id;
					schools[i][chosen_school].m_current_pupils++;

					break;
				}
			}
		}
	}
}

void PopulationGenerator::makeWork(const map<uint, uint>& age_map, Population& pop) {
	auto work_config = m_props.get_child("POPULATION.WORK");
	double employment_rate = work_config.get<double>("AMOUNT.<xmlattr>.fraction") / 100;

	uint max_age = m_props.get<uint>("POPULATION.AGES.<xmlattr>.max");

	MinMax employment_age;
	employment_age.min = work_config.get<uint>("AMOUNT.<xmlattr>.minAge");
	employment_age.max = work_config.get<uint>("AMOUNT.<xmlattr>.maxAge");

	MinMax company_size;
	company_size.min = work_config.get<uint>("COMPANYSIZE.<xmlattr>.min");
	company_size.max = work_config.get<uint>("COMPANYSIZE.<xmlattr>.max");

	/// the use of this variable as a data member is obsolete
	/// i thought every cluster (work, household) had to have a different id
	/// TODO refactor this later
	m_cluster_id = 1;

	uint total = 0;
	/// See how many companies you need
	vector<SimplePerson*> employed_people;
	for (SimplePerson& person: pop.all) {
		if (person.m_age >= employment_age.min and person.m_age <= employment_age.max) {
			/// You're able to work from this age
			/// Note that we allow to work and study at the same time!
			/// Now look at the employment rate
			AliasDistribution dist = AliasDistribution({employment_rate, 1.0 - employment_rate});
			if (dist(m_rng) == 0) {
				employed_people.push_back(&person);
			}
			total++;
		}
	}

	while (employed_people.size() > 0) {
		uint current_company_size = m_rng() % (company_size.max - company_size.min) + company_size.min;
		for (uint i = 0U; i < current_company_size; i++) {
			if (employed_people.size() != 0) {
				employed_people.back()->m_work_id = m_cluster_id;
				employed_people.pop_back();
			} else {
				break;
			}
		}
		m_cluster_id++;
	}
}



void PopulationGenerator::makeCommunities(const map<uint, uint>& age_map, Population& pop) {
	auto community_config = m_props.get_child("POPULATION.COMMUNITY");
	uint community_size = community_config.get<uint>("<xmlattr>.size");
	double communities_per_person = community_config.get<double>("<xmlattr>.average_per_person");
	uint needed_communities = uint (pop.all.size() * communities_per_person / double(community_size) + 0.5);

	uint pop_with_two_communities = needed_communities * community_size - pop.all.size();
	uint pop_with_one_community = pop.all.size() - pop_with_two_communities;

	double pop_two_communities_fraction = double(pop_with_two_communities) / double(pop.all.size());

	/// Make the communities
	vector<uint> communities;

	m_cluster_id = 1;

	for (uint i = 0U; i < needed_communities; i++) {
		communities.push_back(m_cluster_id);
		m_cluster_id++;		
	}


	/// Assign families to a primary community
	AliasDistribution community_dist = AliasDistribution(vector<double>(communities.size(), 1.0 / communities.size()));
	for (SimpleFamily& family: pop.families) {
		/// Pick a community
		uint community_index = community_dist(m_rng);

		for (uint& index: family) {
			SimplePerson& p = pop.all[index];
			p.m_primary_community = communities.at(community_index);
		}
	}

	/// Assign each individual a secondary community if needed
	AliasDistribution individual_dist = AliasDistribution(
		{pop_two_communities_fraction, 1 - pop_two_communities_fraction});

	for (SimplePerson& p: pop.all) {
		if (individual_dist(m_rng) == 0) {
			/// He has a secondary community
			uint current_community_id = p.m_primary_community;

			while (current_community_id == p.m_primary_community) {
				current_community_id = community_dist(m_rng);
			}

			p.m_secondary_community = current_community_id;
		} else {
			p.m_secondary_community = p.m_primary_community;
		}
	}
}
