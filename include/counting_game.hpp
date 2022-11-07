//
// Created by vanda on 04/10/2021.
//
#pragma once

#include "player.hpp"
#include "server_settings.hpp"
#include "server.hpp"
#include <dpp/dpp.h>
#include <sqlite3.h>


namespace Bot {

    class App;

    class CountingGame {

    public:
        enum Type {
            INVALID,
            CORRECT,
            INCORRECT,
            SAME_USER,
            SAVED
        };

        CountingGame(Server &server, int id = 0);

        ~CountingGame();

        void count(Server &server, const dpp::message_create_t &message);

        void reply(const Type type, Server &server, const dpp::message_create_t &message, double value);

        dpp::snowflake getCountChannel() const;

        void setCountChannel(dpp::snowflake channelID);

        std::vector<const Player *> getRankedPlayers(uint32_t amount, uint32_t startPos) const;

        static bool shouldCount(const std::string_view &input);

        void savePlayer(const Player &player);

        Player &addPlayer(dpp::snowflake id);

        void saveGame();

        void addCommands(dpp::cluster &bot, ServerSettings &settings, Server &server);

        void addSettings(dpp::slashcommand &baseCommand, Server &server);

        void onMessageDelete(Server &server, const dpp::message_delete_t &event);

    private:
        void initDataBase(int id);

        static int populateHashSet(void *countGamePtr, int entries, char **value, char **colName);

        static int setServerValues(void *countGamePtr, int entries, char **value, char **colName);

        void addExtraTables();

        std::string initSettings(Server &server);

        Type addCountToPlayer(Server &server,Player &player, bool correct);

        std::unordered_map<dpp::snowflake, Player> players;
        int id;
        dpp::snowflake channelID;
        dpp::snowflake failRollID;
        dpp::snowflake botSpam;
        uint64_t currentCount;
        Player *lastPlayer;
        uint64_t highestCount = 0;
        uint64_t resetCount = 0;
        uint64_t lastMessage = 0;
        sqlite3 *db;
    };
}




