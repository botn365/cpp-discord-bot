//
// Created by bot on 06.11.21.
//

#pragma once

#include "server_settings.hpp"

#include <utility>
#include <vector>

namespace Bot {
    class App;

    class RollSelector {
    public:
        struct Emote {
            Emote(std::string str);

            Emote(dpp::snowflake id, std::string name) : id{id}, name{std::move(name)} {};

            Emote() = default;

            bool operator==(const Emote &emote) const;

            bool operator==(const dpp::emoji) const;

            std::string getStringId() const;

            dpp::snowflake id;
            std::string name;
        };

        struct Roll {
            dpp::snowflake messageId;
            dpp::snowflake rollId;
            Emote emote;
        };

        explicit RollSelector(ServerSettings &settings);

        void addSettings(dpp::slashcommand &baseCommand, App *app);

        void onMessageReactionAdd(App *app, const dpp::message_reaction_add_t &event);

        void onMessageReactionRemove(App *app, const dpp::message_reaction_remove_t &event);

        void removeAllRollsOfReaction(App *app,dpp::snowflake channelID,Roll &roll);

    private:
        std::vector<Roll> rolls;
    };
}




