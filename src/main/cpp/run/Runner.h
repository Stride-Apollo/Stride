
#include <vector>
#include <string>
#include <map>
#include "sim/SimulatorRunMode.h"

namespace stride {
namespace run {

class Runner {
public:
    Runner(const std::vector<std::string>& overrides_list, const std::string& config,
           const RunMode& mode);

    void run();

private:
    std::map<std::string, std::string> m_overrides;
    std::string m_config;
    RunMode m_mode;
};

}
}
