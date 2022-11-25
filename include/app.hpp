//
// Created by bot on 06.11.21.
//

#pragma once

#include "server_settings.hpp"
#include "roll_selector.hpp"
#include "counting_game.hpp"
#include "voice_handler.hpp"
#include "global_settings.hpp"
#include "server.hpp"

#include <dpp/dpp.h>

namespace Bot {
    class App {
    private:
        using call_back = std::function<void(const dpp::interaction_create_t &)>;
        struct Interaction {
            dpp::slashcommand command;
            call_back callBack;
        };
    public:
        void run();

        void registerCommand(dpp::cluster &bot, ServerSettings &settings, dpp::slashcommand &command,
                             std::function<void(const dpp::interaction_create_t &interaction)> interactionCallBack);

        void registerSetting(dpp::slashcommand baseCommand, dpp::command_option &command,
                             std::function<void(const dpp::interaction_create_t &interaction)> interactionCallBack,
                             int location = 0);

        void registerSettingsModuals(dpp::slashcommand &baseCommand);

        GlobalSettings &getGlobalSettings() const { return *globalSettings;}

        void loadServers();

        std::unordered_map<uint64_t, Interaction> commands;
        std::unordered_map<std::string, call_back> settingCallBacks;
        std::unique_ptr<dpp::cluster> bot;

    private:

        void registerSettings(dpp::cluster &bot, Bot::ServerSettings &settings);

        void batchUploadCommands();

        std::vector<Server> servers;
        std::unique_ptr<GlobalSettings> globalSettings;
        std::vector<dpp::slashcommand> tempCommandVector;
        std::vector<call_back> tempCallBackVector;
    };
}




