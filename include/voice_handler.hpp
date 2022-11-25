//
// Created by bot on 05.01.22.
//

#pragma once

#include "dpp/dpp.h"
#include "server_settings.hpp"

namespace Bot {
    class App;
    class VoiceHandler {
    public:

        void onVoiceStateUpdate(const dpp::voice_state_update_t &event);

        void addSettings(dpp::slashcommand &baseCommand);

    private:
        dpp::snowflake voiceRollId;
        dpp::snowflake AFKVoiceChannel;
        App &app;
    public:
        VoiceHandler(App &app);
    };
}




