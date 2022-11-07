//
// Created by vanda on 18/10/2021.
//

#include "../include/server_settings.hpp"
#include "../include/load_operators.hpp"
#include "../include/util.hpp"

#include <fstream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace Bot {
    ServerSettings::ServerSettings(std::string fileString) {
        auto settings = Bot::Util::iFileToString(fileString);
        doc.Parse(settings.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("failed to parse bot json\n CANNOT START BOT");
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
        saveLocation = std::move(fileString);
        if (shouldSave) {
            save();
        }
    }

    void ServerSettings::save() {
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

    const char *ServerSettings::getToken() {
        return doc["bot_token"].GetString();
    }

    const char *ServerSettings::getCountDBLocation() {
        return doc["count_game_db_location"].GetString();
    }

    uint64_t ServerSettings::getServerId() {
        return doc["server_id"].GetUint64();
    }

    void ServerSettings::setServerID(uint64_t serverId) {
        doc["server_id"].SetUint64(serverId);
    }

    std::vector<dpp::command_permission> ServerSettings::getCommandPermissions(std::string &commandName) {
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

