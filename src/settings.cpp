//
// Created by vanda on 18/10/2021.
//

#include "../include/settings.hpp"
#include <rapidjson/document.h>
#include <fstream>
#include "../include/load_operators.hpp"

namespace Bot {
    Settings::Settings(std::string file) {
        auto settings = LoadOperators::fileToString(file);
        rapidjson::Document doc;
        doc.Parse(settings.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("failed to parse json");
        }
        try {
            token = doc["bot_token"].GetString();
            countingGameDBLocation = doc["count_game_db_location"].GetString();
            unicodeTranslationLocation = doc["unicode_translation"].GetString();
            serverId = doc["server_id"].GetInt64();
        } catch (std::exception &e) {
            throw std::runtime_error("json does not find correct values");
        }
    }

    const std::string &Settings::getToken() {
        return token;
    }

    const std::string &Settings::getCountDBLocation() {
        return countingGameDBLocation;
    }

    const std::string &Settings::getUnicodeTranslationLocation() {
        return unicodeTranslationLocation;
    }

    const uint64_t Settings::getServerId() {
        return serverId;
    }
}

