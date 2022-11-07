//
// Created by bot on 10/12/22.
//

#include <fstream>
#include "../include/global_settings.hpp"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "../include/load_operators.hpp"
#include "../include/util.hpp"

namespace Bot {
    GlobalSettings::GlobalSettings(std::string fileString) {
        auto settings = Bot::Util::iFileToString(fileString);
        doc.Parse(settings.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("failed to parse json");
        }
        bool shouldSave = false;
        bool error = false;
        if (!doc.HasMember("bot_token") || !doc["bot_token"].IsString()) {
            rapidjson::Value token;
            token.SetString("");
            doc.AddMember("bot_token",token,doc.GetAllocator());
            shouldSave = true;
            std::cout<<"NO TOKEN FOUND IN SETTINGS!!!\n generated settings file in ";
            error = true;
        }
        if (!doc.HasMember("servers") || !doc["servers"].IsArray()) {
            rapidjson::Value servers;
            servers.SetArray();
            doc.AddMember("servers",servers,doc.GetAllocator());
            shouldSave = true;
        }
        if (!doc.HasMember("unicode_translation")) {
            rapidjson::Value unicode;
            unicode.SetString("unicodeToNumber.json");
            doc.AddMember("unicode_translation",unicode,doc.GetAllocator());
            shouldSave = true;
            std::cout<<"No location for name translation fileString found in settings. Putting it in default location. ~/unicodeToNumber.json\n";
        }
        saveLocation = std::move(fileString);
        if (shouldSave) {
            save();
        }
        if (error) throw std::runtime_error("loading bot settings failed\n CAN NOT START BOT!");
    }

    const char *GlobalSettings::getToken() {
        return doc["token"].GetString();
    }

    std::vector<std::string> GlobalSettings::getServers() {
        std::vector<std::string> servers;
        for (auto &server_settings : doc["servers"].GetArray()) {
            servers.emplace_back(server_settings.GetString());
        }
        return servers;
    }

    void GlobalSettings::save() {
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

    const char *GlobalSettings::getUnicodeTranslationLocation() {
        return doc["unicode_translation"].GetString();
    }
} // Bot