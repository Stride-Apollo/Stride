#pragma once

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <gtest/gtest.h>

namespace Tests {

class Hdf5Base : public ::testing::TestWithParam<unsigned int> {
public:
	const boost::property_tree::ptree getConfigTree() const;
};

}

