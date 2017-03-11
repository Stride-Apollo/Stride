#include "PopulationGenerator.h"

#include <boost/property_tree/xml_parser.hpp>

using namespace stride;
using namespace popgen;
using namespace util;
using namespace boost::property_tree;
using namespace xml_parser;

uniform_real_distribution<double> real01 = uniform_real_distribution<double>(0, 1);

bool SimplePerson::hasCommunitiesLeft() {
	return (m_primary_community != 0) and (m_secondary_community != 0);
}

std::ostream& operator<<(std::ostream& os, const SimplePerson& p) {
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

SimplePerson::SimplePerson(uint age): m_age(age) {}


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
}


Population PopulationGenerator::generate() {
	random_device rd;  // TODO: pick random generator from xml + seed
	Population pop;
	AgeDistribution age_dist(
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.min"),
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.max"),
			m_props.get<uint>("POPULATION.AGES.<xmlattr>.constantUpTo")
	);

	map<uint, vector<uint>> age_map;
	for (uint i=age_dist.getMin(); i<=age_dist.getMax(); i++) {
		age_map[i] = vector<uint>();
	}

	auto add_person = [&](const SimplePerson& p) {
		pop.all.push_back(p);
		age_map[p.m_age].push_back(pop.all.size()-1);

	};

	// Step 1: Families
	// Step 1.a: Families with kids
	uint total_in_fam_kids = 0;
	uint max_in_fam_kids = (uint)(
			(1.0-(m_no_kids_family_size_avg/m_family_size_avg))*double(m_total)
			- (m_family_size_avg-m_no_kids_family_size_avg)/2.0 + 0.5);
	while (total_in_fam_kids <= max_in_fam_kids) {
		uint size = m_some_kids_family_size_dist(rd);
		total_in_fam_kids += size;

		SimplePerson parent1 = SimplePerson(age_dist.get_limited(rd, m_age_parents));
		SimplePerson parent2 = SimplePerson(age_dist.get_limited(rd,
				max(m_age_parents.min, parent1.m_age-m_age_diff_parents_max),
				min(m_age_parents.max, parent1.m_age+m_age_diff_parents_max)));

	}

}


void AgeDistribution::correct(uint total, const map<uint, uint>& age_counts) {

}
