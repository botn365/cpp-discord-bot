//
// Created by bot on 10/6/22.
//

#pragma once

#include <string>

namespace bot::getApplicationStats {
#ifdef __linux__
    std::string getCPUusage();
    std::string getMemoryUsage();
    std::string getUptime();
#endif
}
