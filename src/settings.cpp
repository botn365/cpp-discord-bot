//
// Created by vanda on 18/10/2021.
//

#include "../include/settings.hpp"
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
            loadPermissions(doc);
        } catch (std::exception &e) {
            throw std::runtime_error("json does not find correct values");
        }
    }

    //TODO redo permision handeling
    void Settings::loadPermissions(rapidjson::Document &doc) {
        using p = std::pair<std::string,dpp::command_permission>;
        for (auto &obj : doc["command_peremissions"].GetArray()) {
            dpp::command_permission permission;
            if (obj["is_user"].GetBool()) {
                permission.type = dpp::cpt_user;
            } else {
                permission.type = dpp::cpt_role;
            }
            permission.id = obj["roll_id"].GetInt64();
            permission.permission = obj["allow"].GetBool();
            commandPermisions.insert(p(obj["roll_name"].GetString(),permission));
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

    const dpp::command_permission *Settings::getCommandPermission(std::string &rollName) {
        auto permisson = commandPermisions.find(rollName);
        if (permisson != commandPermisions.end()) {
            return &permisson->second;
        }
        return nullptr;
    }
}

