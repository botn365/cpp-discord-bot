//
// Created by vanda on 18/10/2021.
//

#pragma once

#include <string>
#include <unordered_map>
#include <dpp/dpp.h>
#include <rapidjson/document.h>

namespace Bot {
    class ServerSettings {
    public:

        ServerSettings(std::string fileString);

        const char *getToken();

        const char *getCountDBLocation();

        uint64_t getServerId();

        void setServerID(uint64_t);

        void save();

        std::vector<dpp::command_permission> getCommandPermissions(std::string &commandName);

        rapidjson::Document &getDocument() {return doc;}

    private:
        rapidjson::Document doc;
        std::string saveLocation;
    };
}



