//
// Created by bot on 10/6/22.
//

#include "../include/get_harware_stats.hpp"
#include <stdlib.h>

namespace bot::getApplicationStats {
#ifdef __linux__
    std::string getCPUusage() {

        return std::string();
    }

    std::string getMemoryUsage() {
        return std::string();
    }

    std::string getUptime() {
        return std::string();
    }
#endif
} // bot