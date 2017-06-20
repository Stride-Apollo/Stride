#pragma once

/**
 * @file
 * Header for the Influence class
 */

#include <deque>
#include <cmath>

using namespace std;

namespace stride {

using uint = unsigned int;

class Influence {
public:
	Influence(uint size, double speed): m_deque(deque<uint>(size, 0)), m_speed(speed) {}

	void addRecord(uint record) {
		m_deque.push_front(record);
		m_deque.pop_back();
	}

	double getInfluence() const {
		uint score = 0;

		for (const auto& current_score: m_deque) {
			score += current_score;
		}

		if (score <= 1)
			return 0.0;
		else
			return m_speed * log10(score);
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
};

}
