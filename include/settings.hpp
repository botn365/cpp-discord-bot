//
// Created by vanda on 18/10/2021.
//

#pragma once

#include <string>
#include <unordered_map>
#include <dpp/dpp.h>
#include <rapidjson/document.h>

namespace Bot {
    class Settings {
    public:

        Settings(std::string file);

        const char *getToken();

        const char *getCountDBLocation();

        const char *getUnicodeTranslationLocation();

        const uint64_t getServerId();

        void save();

        std::vector<dpp::command_permission> getCommandPermissions(std::string &commandName);

        rapidjson::Document &getDocument() {return doc;}

    private:
        rapidjson::Document doc;
        std::string saveLocation;
    };
}



