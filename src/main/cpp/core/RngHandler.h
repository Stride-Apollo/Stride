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
 * Header for the ContactHandler class.
 */

#include "util/Random.h"
#include "math.h"
#include <iostream>

namespace stride {

/**
 * Processes the contacts between persons and determines whether transmission occurs.
 */
class RngHandler {
public:
	/// Constructor sets the transmission rate and random number generator.
	RngHandler(unsigned int seed, unsigned int stream_count, unsigned int id)
			: m_rng(seed) {
		m_rng.split(stream_count, id);
	}

	/// Gets the seed (for saving purposes)
	unsigned int getSeed() const {
		return m_seed;
	}

	/// Convert rate into probability
	double rateToProbability(double rate) {
		return 1 - exp(-rate);
	}

	/// Check if two individuals have transmission.
	bool hasTransmission(double contact_rate, double transmission_rate) {
		return m_rng.nextDouble() < rateToProbability(transmission_rate * contact_rate);
	}

	/// Check if two individuals have contact.
	bool hasContact(double contact_rate) {
		return m_rng.nextDouble() < rateToProbability(contact_rate);
	}

	friend std::ostream& operator<<(std::ostream& os, const RngHandler& handler) {
		os << handler.m_rng;
		return os;
	} 

private:
	unsigned int m_seed;
	util::Random m_rng;                        ///< Random number engine.
};

}

