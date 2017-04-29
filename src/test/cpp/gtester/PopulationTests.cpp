
#include <iostream>

#include <gtest/gtest.h>

#include "pop/Population.h"

using namespace std;
using namespace stride;
using namespace util;
using namespace ::testing;

namespace Tests {

// I'll only care about id and on_vacation here
Simulator::PersonType P(unsigned int id, bool on_vacation = false) {
	return Simulator::PersonType(id, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, on_vacation);
}

class PopulationTest : public ::testing::Test {
public:
	virtual ~PopulationTest() {}

	Population pop;
};

#define VARIOUS_POPULATION_TESTS(fixture, max) \
TEST_F(fixture, fixture ## Simple) { \
	auto it = pop.begin(); \
	for (int i=0; i<max; i++) { \
		EXPECT_EQ(i, (*it).getId()); \
		it++; \
	} \
} \
\
TEST_F(fixture, fixture ## SimpleAuto) { \
	int i = 0; \
	for (auto& it: pop) { \
		EXPECT_EQ(i, it.getId()); \
		i++; \
	} \
	EXPECT_EQ(i, max); \
} \
\
TEST_F(fixture, fixture ## ConstAuto) { \
	int i = 0; \
	for (const auto& it: pop) { \
		EXPECT_EQ(i, it.getId()); \
		i++; \
	} \
	EXPECT_EQ(i, max); \
}


class BitOfBothPopulationTest : public PopulationTest {
public:
	virtual void SetUp() {
		pop.m_original.push_back(P(0));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(1));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(2));
		pop.m_original.push_back(P(3));
		pop.m_original.push_back(P(999, true));
		pop.m_visitors.add(3, P(4));
		pop.m_visitors.add(10, P(5, true));
		pop.m_visitors.add(10, P(6, true));
		pop.m_visitors.add(12, P(7));
		pop.m_visitors.add(12, P(8));
	}
};

VARIOUS_POPULATION_TESTS(BitOfBothPopulationTest, 9)


class OnlyOriginalPopulationTest : public PopulationTest {
public:
	virtual void SetUp() {
		pop.m_original.push_back(P(0));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(1));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(2));
		pop.m_original.push_back(P(3));
		pop.m_original.push_back(P(999, true));
	}
};

VARIOUS_POPULATION_TESTS(OnlyOriginalPopulationTest, 4)


class OnlyRemotePopulationTest : public PopulationTest {
public:
	virtual void SetUp() {
		pop.m_visitors.add(3, P(0));
		pop.m_visitors.add(10, P(1, true));
		pop.m_visitors.add(10, P(2, true));
		pop.m_visitors.add(12, P(3));
		pop.m_visitors.add(12, P(4));
	}
};

VARIOUS_POPULATION_TESTS(OnlyRemotePopulationTest, 5)


class EmptyPopulationTest : public PopulationTest {
public:
	virtual void SetUp() {
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(999, true));
		pop.m_original.push_back(P(999, true));
	}
};

VARIOUS_POPULATION_TESTS(EmptyPopulationTest, 0)

// Truly empty
VARIOUS_POPULATION_TESTS(PopulationTest, 0)


}