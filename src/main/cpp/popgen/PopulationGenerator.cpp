#include "PopulationGenerator.h"

#include <boost/property_tree/xml_parser.hpp>

#include <algorithm>

using namespace stride;
using namespace popgen;
using namespace util;
using namespace boost::property_tree;
using namespace xml_parser;

uniform_real_distribution<double> popgen::real01 = uniform_real_distribution<double>(0, 1);

bool SimplePerson::hasCommunitiesLeft() {
	return (m_primary_community != 0) and (m_secondary_community != 0);
}

std::ostream& popgen::operator<<(std::ostream& os, const SimplePerson& p) {
	p.print(os);
	return os;
}

void SimplePerson::print(std::ostream& os) const {
	assert(m_household_id != 0);  // everyone is part of a family!

	// "age","household_id","school_id","work_id","primary_community","secondary_community"
	os << m_age << "," << m_household_id << ","
	   << m_school_id << "," << m_work_id << ","
	   << m_primary_community << "," << m_secondary_community << "\n";
}

std::ostream& popgen::operator<<(std::ostream& os, const Population& p) {
	os << "\"age\",\"household_id\",\"school_id\",\"work_id\",\"primary_community\",\"secondary_community\"\n";
	for (const SimplePerson& person: p.all) {
		person.print(os);
	}
	return os;
}

SimplePerson::SimplePerson(uint age, uint family_id): m_age(age), m_household_id(family_id) {}


PopulationGenerator::PopulationGenerator(const string& filename) {
	read_xml(filename, m_props, trim_whitespace | no_comments);
	m_total = m_props.get<uint>("POPULATION.<xmlattr>.total");

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


Population PopulationGenerator::generate() {
	mt19937 rd;  // TODO: pick random generator from xml + seed
	rd.seed(123);
	Population pop;
	AgeDistribution age_dist(
			m_total,
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.min"),
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.max"),
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.constantUpTo")
	);

	map<uint, uint> age_map;
	for (uint i=age_dist.getMin(); i<=age_dist.getMax(); i++) {
		age_map[i] = 0;
	}

	auto add_person = [&](const SimplePerson& p) {
		pop.all.push_back(p);
		age_map[p.m_age]++;
	};

	// Step 1: Families
	// Step 1.a: Families with kids
	uint total_in_fam_kids = 0;
	uint max_in_fam_kids = (uint)(
			(1.0-(m_no_kids_family_size_avg/m_family_size_avg))*double(m_total)
			- (m_family_size_avg-m_no_kids_family_size_avg)/2.0 + 0.5);
	uint family_id = 1;
	while (total_in_fam_kids <= max_in_fam_kids) {
		uint size = m_some_kids_family_size_dist(rd);
		assert(size >= 3);

		uint parent1 = age_dist.get_limited(rd, m_age_parents);
		uint parent2 = age_dist.get_limited(rd,
				max(m_age_parents.min, parent1-m_age_diff_parents_max),
				min(m_age_parents.max, parent1+m_age_diff_parents_max));

		uint parent_min_age = min(parent1, parent2);
		uint max_age = min(parent_min_age-m_age_diff_parents_kids_min, m_age_kids.max);
		uint num_children = size - 2;

		// Check for highest possible amount of children in given range
		uint possible = double(min(max_age-m_age_kids.min, m_age_diff_kids.max))/m_age_diff_kids.min;
		if (num_children > possible) continue;

		vector<uint> children_ages(num_children);
		while (true) {
			for (uint i=0; i<num_children; i++) {
				children_ages[i] = age_dist.get_limited(rd, m_age_kids.min, max_age);
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

		add_person(SimplePerson(parent1, family_id));
		add_person(SimplePerson(parent2, family_id));
		for (uint& a: children_ages) add_person(SimplePerson(a, family_id));
		total_in_fam_kids += size;
		family_id++;
		age_dist.correct(pop.all.size(), age_map);
	}

	// Step 1.b Families without kids
	while (pop.all.size() <= m_total-(m_no_kids_family_size_avg/2.0)) {
		uint size = m_no_kids_family_size_dist(rd);
		vector<uint> ages(size);
		MinMax limits = m_age_parents;
		for (uint i=0; i<size; i++) {
			ages[i] = age_dist.get_limited(rd, limits);
			limits.min = max(limits.min, ages[i] - m_age_diff_parents_max);
			limits.max = min(limits.max, ages[i] + m_age_diff_parents_max);
		}

		for (uint& a: ages) add_person(SimplePerson(a, family_id));
		family_id++;
		age_dist.correct(pop.all.size(), age_map);
	}

	// TODO:
	//  - schools
	//  - work
	//  - communities

	return pop;
}


void AgeDistribution::correct(uint total, const map<uint, uint>& age_counts) {

}

