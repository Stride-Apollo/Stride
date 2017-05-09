#pragma once

#include <string>
#include <vector>
#include <utility>

using namespace std;

namespace stride {
namespace util {

using uint = unsigned int;

class TransportFacilityReader {
public:
	vector<pair<string, string> > readFacilities(string filename);

private:
	pair<string, string> parseFacility(string row);
};

}
}