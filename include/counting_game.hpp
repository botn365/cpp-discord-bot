//
// Created by vanda on 04/10/2021.
//
#pragma once

#include "player.hpp"
#include "settings.hpp"
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

        CountingGame(App *app, int id = 0);

        ~CountingGame();

        void count(App *app, const dpp::message_create_t &message);

        void reply(const Type type, App *app, const dpp::message_create_t &message, double value);

        dpp::snowflake getCountChannel() const;

        void setCountChannel(dpp::snowflake channelID);

        std::vector<const Player *> getRankedPlayers(uint32_t amount, uint32_t startPos) const;

        static bool shouldCount(const std::string_view &input);

        void savePlayer(const Player &player);

        Player &addPlayer(dpp::snowflake id);

        void saveGame();

        void addCommands(dpp::cluster &bot, Settings &settings, App *app);

        void addSettings(dpp::slashcommand &baseCommand, App *app);

    private:
        void initDataBase(int id);

        static int populateHashSet(void *countGamePtr, int entries, char **value, char **colName);

        static int setServerValues(void *countGamePtr, int entries, char **value, char **colName);

        void addExtraTables();

        std::string initSettings(App *app);

        Type addCountToPlayer(App *bot,Player &player, bool correct);

        std::unordered_map<dpp::snowflake, Player> players;
        int id;
        dpp::snowflake channelID;
        dpp::snowflake failRollID;
        dpp::snowflake botSpam;
        uint64_t currentCount;
        Player *lastPlayer;
        uint64_t highestCount = 0;
        uint64_t resetCount = 0;
        sqlite3 *db;
    };
}




