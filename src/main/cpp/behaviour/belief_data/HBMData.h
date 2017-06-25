/*
 * HBMData.h
 *
 *  Created on: Apr 19, 2017
 *      Author: elise
 */

#pragma once

namespace stride {

enum class VariableState {
	Low = 0U, High = 1U
};

class HBMData {

public:
	HBMData() :
			m_perceived_severity(0), m_perceived_susceptibility(0),
			m_perceived_benefit(1), m_perceived_barriers(1) {}

	VariableState getPerceivedSeverity() const {
		return m_perceived_severity;
	}

	VariableState getPerceivedSusceptibility() const {
		return m_perceived_susceptibility;
	}

	VariableState getPerceivedBenefit() const {
		return m_perceived_benefit;
	}

	VariableState getPerceivedBarriers() const {
		return m_perceived_barriers;
	}

	void MeetAdopted() {
		m_num_contacts_adopted++;
		m_num_contacts++;
	}

	void MeetInfected() {
		m_num_contacts_infected++;
		m_num_contacts++;
	}

	void MeetInfectedAndAdopted() {
		m_num_contacts_adopted++;
		m_num_contacts_infected++;
		m_num_contacts++;
	}

private:
	int m_num_contacts;
	int m_num_contacts_infected;
	int m_num_contacts_adopted;
	VariableState m_perceived_severity;
	VariableState m_perceived_susceptibility;
	VariableState m_perceived_benefit;
	VariableState m_perceived_barriers;
};

}

