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

        const std::string &getToken();

        const std::string &getCountDBLocation();

        const std::string &getUnicodeTranslationLocation();

        const uint64_t getServerId();

        const dpp::command_permission *getCommandPermission(std::string &rollName);

    private:
        void loadPermissions(rapidjson::Document &doc);
        std::string token;
        std::string countingGameDBLocation;
        std::string unicodeTranslationLocation;
        uint64_t serverId;
        std::unordered_map<std::string,dpp::command_permission> commandPermisions;
    };
}



