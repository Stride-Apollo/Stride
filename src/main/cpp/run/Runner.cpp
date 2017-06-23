
#include <exception>
#include "Runner.h"
#include "util/StringUtils.h"

using namespace stride;
using namespace util;
using namespace run;
using namespace std;

Runner::Runner(const vector<string>& overrides_list, const string& config, const RunMode& mode)
        : m_config(config), m_mode(mode) {
    for (const string& kv: overrides_list) {
        vector<string> parts = StringUtils::split(kv, "=");
        if (parts.size() != 2) {
            throw runtime_error(string("Couldn't parse the override ") + kv);
        }

        string key = string("run.") + StringUtils::trim(parts[0]);
        key = StringUtils::replace(key, "$", "<xmlattr>.");
    }
}

void Runner::run() {

}
