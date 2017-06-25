#pragma once

namespace stride {

struct SimulatorStatus {
	SimulatorStatus(int _infected, int _adopted) : infected(_infected), adopted(_adopted) {}

	int infected;
	int adopted;
};

}
