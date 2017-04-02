#pragma once

#include <string>
#include <vector>
#include <fstream>

namespace stride {
namespace popgen {

using namespace std;

using uint = unsigned int;
using FamilyConfig = vector<uint>;

class FamilyParser {
public:
	FamilyParser() {}
	~FamilyParser() {}

	vector<FamilyConfig> parseFamilies(string filename) const;

private:
	FamilyConfig parseFamily(string config) const;

};

}
}