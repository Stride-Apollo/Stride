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

#ifdef USE_HDF5
#include "checkpointing/Hdf5Loader.h"
#include "checkpointing/Hdf5Saver.h"
#endif

namespace stride {

enum class HealthStatus {
	Susceptible = 0U, Exposed = 1U, Infectious = 2U,
	Symptomatic = 3U, InfectiousAndSymptomatic = 4U, Recovered = 5U, Immune = 6U, Null
};


class Health {
public:
	///
	Health(unsigned int start_infectiousness, unsigned int start_symptomatic,
		   unsigned int time_infectious, unsigned int time_symptomatic);

	///
	HealthStatus getHealthStatus() const { return m_status; }

	///
	unsigned int getEndInfectiousness() const { return m_end_infectiousness; }

	///
	unsigned int getEndSymptomatic() const { return m_end_symptomatic; }

	///
	unsigned int getStartInfectiousness() const { return m_start_infectiousness; }

	///
	unsigned int getStartSymptomatic() const { return m_start_symptomatic; }

	///
	bool isImmune() const { return m_status == HealthStatus::Immune; }

	///
	bool isInfected() const {
		return m_status == HealthStatus::Exposed
			   || m_status == HealthStatus::Infectious
			   || m_status == HealthStatus::InfectiousAndSymptomatic
			   || m_status == HealthStatus::Symptomatic;
	}

	///
	bool isInfectious() const {
		return m_status == HealthStatus::Infectious
			   || m_status == HealthStatus::InfectiousAndSymptomatic;
	}

	///
	bool isRecovered() const { return m_status == HealthStatus::Recovered; }

	/// Is this person susceptible?
	bool isSusceptible() const { return m_status == HealthStatus::Susceptible; }

	/// Is this person symptomatic?
	bool isSymptomatic() const {
		return m_status == HealthStatus::Symptomatic
			   || m_status == HealthStatus::InfectiousAndSymptomatic;
	}

	/// Set immune to true.
	void setImmune();

	/// Start the infection.
	void startInfection();

	/// Stop the infection.
	void stopInfection();

	/// Update progress of the disease.
	void update();

private:
	/// Get the disease counter.
	unsigned int getDiseaseCounter() const { return m_disease_counter; }

	/// Increment disease counter.
	void incrementDiseaseCounter() { m_disease_counter++; }

	/// Reset the disease counter.
	void resetDiseaseCounter() { m_disease_counter = 0U; }

private:
	unsigned int m_disease_counter;              ///< The disease counter.
	HealthStatus m_status;                       ///< The current status of the person w.r.t. the disease.

	unsigned int m_start_infectiousness;         ///< Days after infection to become infectious.
	unsigned int m_start_symptomatic;            ///< Days after infection to become symptomatic.
	unsigned int m_end_infectiousness;           ///< Days after infection to end infectious state.
	unsigned int m_end_symptomatic;              ///< Days after infection to end symptomatic state.

private:
	#ifdef HDF5_USED
		friend class Hdf5Loader;
		friend class Hdf5Saver;
	#endif
};

}
