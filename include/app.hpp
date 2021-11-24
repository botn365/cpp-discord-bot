//
// Created by bot on 06.11.21.
//

#pragma once

#include "settings.hpp"
#include "roll_selector.hpp"
#include "counting_game.hpp"

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

        void registerCommand(dpp::cluster &bot, Settings &settings, dpp::slashcommand &command,
                             std::function<void(const dpp::interaction_create_t &interaction)> interactionCallBack);

        void registerSetting(dpp::slashcommand baseCommand, dpp::command_option &command,
                             std::function<void(const dpp::interaction_create_t &interaction)> interactionCallBack,
                             int location = 0);

        void registerSettingsModuals(dpp::slashcommand &baseCommand);

        std::unordered_map<uint64_t, Interaction> commands;
        std::unordered_map<std::string, call_back> settingCallBacks;
        std::unique_ptr<dpp::cluster> bot;
        std::unique_ptr<Settings> settings;
        std::unique_ptr<CountingGame> countingGame;

    private:

        void registerSettings(dpp::cluster &bot, Bot::Settings &settings);

        void batchUploadCommands();

        std::vector<dpp::slashcommand> tempCommandVector;
        std::vector<call_back> tempCallBackVector;

        std::unique_ptr<RollSelector> rollSelector;
    };
}




