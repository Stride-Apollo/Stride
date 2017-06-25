/*
 * RNG.h
 *
 *  Created on: Apr 20, 2017
 *      Author: elise
 */

#pragma once

#include <trng/mrg2.hpp>
#include <trng/uniform01_dist.hpp>

namespace stride {

class RNG {
private:
	RNG() {
		m_uniform_dist = trng::uniform01_dist<double>();
	}

	RNG(const RNG&);

	RNG& operator=(const RNG&);

public:
	static RNG& getInstance() {
		static RNG instance;
		return instance;
	}

	double nextDouble() {
		return m_uniform_dist(m_engine);
	}

private:
	trng::mrg2 m_engine;         ///< The random number engine.
	trng::uniform01_dist<double> m_uniform_dist;   ///< The random distribution.
};

}
