#pragma once

/**
 * @file
 * Header for the Influence class
 */

#include <deque>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;

namespace stride {

using uint = unsigned int;

class ClusterSaver;

class Influence {
public:
	Influence(uint size, double speed, double minimum) : m_deque(deque<uint>(size, 0)), m_speed(speed),
														 m_minimum(minimum) {
		if (minimum <= 0.0) {
			throw runtime_error(string(__func__) + string("Influence minimum <= 0.0"));
		}
	}

	void addRecord(uint record) {
		m_deque.push_front(record);
		m_deque.pop_back();
	}

	void addToFront(uint amount) {
		m_deque.front() = m_deque.front() + amount;
	}

	double getInfluence() const {
		uint score = 0;

		for (const auto& current_score: m_deque) {
			score += current_score;
		}

		if (score <= 1)
			return m_minimum;
		else
			return max(m_speed * log10(score), m_minimum);
	}

	uint getScore() const {
		uint score = 0;

		for (const auto& current_score: m_deque) {
			score += current_score;
		}

		return score;
	}

private:
	Influence();

	deque<uint> m_deque;
	double m_speed;
	double m_minimum;

	friend class ClusterSaver;
};

}
