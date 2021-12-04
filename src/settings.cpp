//
// Created by vanda on 18/10/2021.
//

#include "../include/settings.hpp"
#include "../include/load_operators.hpp"

#include <fstream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace Bot {
    Settings::Settings(std::string file) {
        auto settings = LoadOperators::iFileToString(file);
        doc.Parse(settings.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("failed to parse json");
        }
        bool shouldSave = false;
        if (!doc.HasMember("bot_token")) {
            rapidjson::Value token;
            token.SetString("");
            doc.AddMember("bot_token",token,doc.GetAllocator());
            shouldSave = true;
            std::cout<<"NO TOKEN FOUND IN SETTINGS!!!\n";
        }
        if (!doc.HasMember("server_id")) {
            rapidjson::Value serverId;
            serverId.SetInt64(0);
            doc.AddMember("server_id",serverId,doc.GetAllocator());
            shouldSave = true;
            std::cout<<"NO SERVER ID FOUND!!! \n";
        }
        if (!doc.HasMember("unicode_translation")) {
            rapidjson::Value unicode;
            unicode.SetString("unicodeToNumber.json");
            doc.AddMember("unicode_translation",unicode,doc.GetAllocator());
            shouldSave = true;
            std::cout<<"No location for name translation file found in settings. Putting it in default location. ~/unicodeToNumber.json\n";
        }
        saveLocation = std::move(file);
        if (shouldSave) {
            save();
        }
    }

    void Settings::save() {
        std::ofstream stream(saveLocation,std::ios::trunc);
        if (stream.is_open()) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            auto newJsonString = buffer.GetString();
            stream.write(newJsonString,buffer.GetLength());
            stream.close();
        }
    }

    const char *Settings::getToken() {
        return doc["bot_token"].GetString();
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
            throw std::runtime_error("name translation location not found");
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
                        list.push_back(permission);
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

