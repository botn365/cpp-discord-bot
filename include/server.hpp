//
// Created by bot on 10/12/22.
//

#pragma once

#include <memory>
#include "server_settings.hpp"
#include "counting_game.hpp"
#include "roll_selector.hpp"
#include "voice_handler.hpp"

namespace Bot {
    class App;
    class Server {
    public:
        explicit Server(int id,App &app);

        explicit Server(std::string folder, App &app);

        void init();

        void registerCallbacks();

        [[nodiscard]] App &getApp() const {return app;}

        [[nodiscard]] rapidjson::Document &getDocument() const {return settings->getDocument();}

        [[nodiscard]] const std::string  &getFolder() const {return folder;}

        [[nodiscard]] dpp::cluster &getBot() const;

        [[nodiscard]] dpp::snowflake id() const {return serverID;}

        void save() {settings->save();};

    public:

    private:

        App &app;
        std::unique_ptr<ServerSettings> settings;
        //std::unique_ptr<CountingGame> countingGame;
        std::unique_ptr<RollSelector> rollSelector;
        std::unique_ptr<VoiceHandler> voiceHandler;
        dpp::snowflake serverID;
        std::string folder;
    };
} // Bot
