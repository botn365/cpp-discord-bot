//
// Created by bot on 06.11.21.
//

#pragma once
#include "settings.hpp"
#include "roll_selector.hpp"

#include <dpp/dpp.h>

namespace Bot {
    class App {
    public:
        void run();

        void registerCommand(dpp::cluster &bot, Settings &settings, dpp::slashcommand &command,
                             std::function<void(const dpp::interaction_create_t &interaction)> interactionCallBack);

        void registerSetting(dpp::slashcommand baseCommand,dpp::command_option &command,
                             std::function<void(const dpp::interaction_create_t &interaction)> interactionCallBack, int location = 0);

        std::unordered_map<uint64_t,dpp::slashcommand> commands;
        std::unordered_map<uint64_t,std::function<void(const dpp::interaction_create_t &interaction)>> interactions;
        std::unordered_map<std::string,std::function<void(const dpp::interaction_create_t &interaction)>> settingCallBacks;
        std::unique_ptr<dpp::cluster> bot;
        std::unique_ptr<Settings> settings;

    private:
        void registerSettings(dpp::cluster &bot, Bot::Settings &settings);
        std::unique_ptr<RollSelector> rollSelector;
    };
}




