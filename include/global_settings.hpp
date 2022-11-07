//
// Created by bot on 10/12/22.
//


#include <vector>
#include "rapidjson/document.h"

namespace Bot {
    class GlobalSettings {
    public:
        GlobalSettings(std::string fileString);

        const char *getToken();

        std::vector<std::string> getServers();

        void save();

        const char *getUnicodeTranslationLocation();

    private:
        rapidjson::Document doc;
        std::string saveLocation;
    };

} // Bot
