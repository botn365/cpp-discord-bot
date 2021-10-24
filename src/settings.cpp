//
// Created by vanda on 18/10/2021.
//

#include "../include/settings.hpp"
#include <fstream>
#include "../include/load_operators.hpp"

namespace Bot {
    Settings::Settings(std::string file) {
        auto settings = LoadOperators::fileToString(file);
        doc.Parse(settings.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("failed to parse json");
        }
    }

    const char *Settings::getToken() {
        try {
            return doc["bot_token"].GetString();
        } catch (std::exception &e) {
            throw std::runtime_error("token not found");
        }
    }

    const char *Settings::getCountDBLocation() {
        try {
            return doc["count_game_db_location"].GetString();
        } catch (std::exception &e) {
            throw std::runtime_error("db location not found");
        }
    }

    const char *Settings::getUnicodeTranslationLocation() {
        try {
            return doc["unicode_translation"].GetString();
        } catch (std::exception &e) {
            throw std::runtime_error("unicode translation location not found");
        }
    }

    const uint64_t Settings::getServerId() {
        try {
            return doc["server_id"].GetInt64();
        } catch (std::exception &e) {
            throw std::runtime_error("server id not found");
        }
    }

    std::vector<dpp::command_permission> Settings::getCommandPermissions(std::string &commandName) {
        try {
            std::vector<dpp::command_permission> list;
            for (auto &data : doc["slash_commands"].GetArray()) {
                if (data["name"].GetString() == commandName) {
                    for (auto &permData : data["permissions"].GetArray()) {
                        dpp::command_permission permission{};
                        if(permData["is_user"].GetBool()) {
                            permission.type = dpp::cpt_user;
                        } else {
                            permission.type = dpp::cpt_role;
                        }
                        permission.permission = permData["allow"].GetBool();
                        permission.id = permData["id"].GetInt64();
                    }
                    break;
                }
            }
            return list;
        } catch (std::exception &e) {
            std::cout<<"ERROR Failed to get command permissions"<<"\n";
            std::cout<<e.what()<<"\n";
            return std::vector<dpp::command_permission>{};
        }
    }
}

