//
// Created by vanda on 18/10/2021.
//

#pragma once

#include <string>

namespace Bot {
    class Settings {
    public:
        Settings(std::string file);

        const std::string &getToken();

        const std::string &getCountDBLocation();

        const std::string &getUnicodeTranslationLocation();

        const uint64_t getServerId();

    private:
        std::string token;
        std::string countingGameDBLocation;
        std::string unicodeTranslationLocation;
        uint64_t serverId;
    };
}



