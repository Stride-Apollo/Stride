#pragma once
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
 * Header for the Random Number Generator class.
 */

#include <limits>

#include <trng/mrg2.hpp>
#include <trng/uniform01_dist.hpp>
#include <trng/uniform_int_dist.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace stride {
namespace util {

inline double rateToProbability(double rate) {
	return 1 - exp(-rate);
}

/// The random number generator.
class Random {
public:
	Random(const unsigned long seed = 0) {
		m_engine.seed(seed);
		m_uniform_dist = trng::uniform01_dist<double>();
	}

	double nextDouble() {
		return m_uniform_dist(m_engine);
	}

	/// Get random unsigned int from [0, max[.
	unsigned int operator()(unsigned int max = std::numeric_limits<unsigned int>::max()) {
		trng::uniform_int_dist dis(0, max);
		return dis(m_engine);
	}

	/// Check if two individuals have contact.
	bool hasContact(double contact_rate) {
		return nextDouble() < rateToProbability(contact_rate);
	}

	/// Check if two individuals have transmission.
	bool hasTransmission(double transmission_rate)  {
		return nextDouble() < rateToProbability(transmission_rate);
	}

	bool hasContactAndTransmission(double contact_rate, double transmission_rate) {
		return nextDouble() < rateToProbability(transmission_rate * contact_rate);
	}

	void setState(std::string state) {
		std::stringstream ss;
		ss.str(state);
		ss >> m_engine;
	}

	friend std::ostream& operator<<(std::ostream& os, const Random& random) {
		os << random.m_engine;
		return os;
	}

private:
	trng::mrg2 m_engine;         ///< The random number engine.
	trng::uniform01_dist<double> m_uniform_dist;   ///< The random distribution.
};


}
}

