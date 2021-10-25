//
// Created by vanda on 04/10/2021.
//

#include "../include/counting_game.hpp"
#include "../include/string_calculator.hpp"

#include <sstream>
#include <string>
#include <cmath>
#include <iomanip>


namespace Bot {
    Bot::CountingGame::CountingGame(std::string dataBase, int id) :
            channelID{0}, currentCount{0}, lastPlayer{nullptr}, id{id} {
        int error = sqlite3_open(dataBase.c_str(), &db);
        if (error) {
            std::cout << "failed to open dataBase" << sqlite3_errmsg(db) << "\n";
            sqlite3_close(db);
            throw std::runtime_error("failed to open dataBase");
        }
        initDataBase(id);
    }

    CountingGame::~CountingGame() {
        sqlite3_close(db);
    }

    void CountingGame::initDataBase(int id) {
        char *msgError;
        int error = sqlite3_exec(db,
                                 "CREATE TABLE PLAYER("
                                 "SNOWFLAKE INT PRIMARY KEY NOT NULL,"
                                 "TOTAL_CORRECT INT NOT NULL,"
                                 "TOTAL_FAILED INT NOT NULL,"
                                 "HIGHEST_COUNT INT NOT NULL);",
                                 NULL, 0, &msgError);
        if (error !=
            SQLITE_OK) { //if it erros that should mean the table already exists so then pull data from db to memory
            sqlite3_free(msgError);
            //load player data
            error = sqlite3_exec(db, "SELECT * FROM PLAYER;", CountingGame::populateHashSet, this, &msgError);
            if (error != SQLITE_OK) {
                std::cout << "on players init" << msgError << "\n";
                sqlite3_free(msgError);
                throw std::runtime_error("failed to load player data");
            }
            std::stringstream ss;
            //load game data
            ss << "SELECT * FROM GAME WHERE ID = " << id << ";";
            error = sqlite3_exec(db, ss.str().c_str(), CountingGame::setServerValues, this, &msgError);
            if (error != SQLITE_OK) {
                std::cout << "on game init" << msgError << "\n";
                sqlite3_free(msgError);
                throw std::runtime_error("failed to load game data");
            }
        } else { //if it does not error that means its a fresh db so create the tables we need
            int ec = sqlite3_exec(db,
                                  "CREATE TABLE GAME("
                                  "ID INT PRIMARY KEY NOT NULL,"
                                  "CHANNEL_ID INT NOT NULL,"
                                  "CURRENT_COUNT INT NOT NULL,"
                                  "LAST_PLAYER INT NOT NULL);",
                                  NULL, NULL, &msgError);
            if (ec != SQLITE_OK) {
                std::cout << "crated player table but faild to create server database could be mallformed" << "\n"
                          << msgError << "\n";
                throw std::runtime_error("error on database init");
            } else {
                std::stringstream ss;
                ss << "INSERT INTO GAME VALUES(" << id << "," << 0 << "," << 0 << "," << 0 << ");";
                ec = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &msgError);
                if (ec != SQLITE_OK) {
                    std::cout << "failed to insert game in to db " << msgError << "\n";
                    sqlite3_free(msgError);
                    throw std::runtime_error("failed to insert game data in to DB");
                }
            }
        }
    }

    //callback for loading player data
    int CountingGame::populateHashSet(void *data, int argc, char **value, char **colName) {
        CountingGame *game = (CountingGame *) data;
        uint64_t snowflake = 0;
        uint64_t sucCount = 0;
        uint64_t failCount = 0;
        uint64_t highestCount = 0;
        for (int i = 0; i < argc; i++) {
            if (strcmp(colName[i], "SNOWFLAKE") == 0) {
                snowflake = std::stoll(value[i]);
            } else if (strcmp(colName[i], "TOTAL_CORRECT") == 0) {
                sucCount = std::stoll(value[i]);
            } else if (strcmp(colName[i], "TOTAL_FAILED") == 0) {
                failCount = std::stoll(value[i]);
            } else if (strcmp(colName[i], "HIGHEST_COUNT") == 0) {
                highestCount = std::stoll(value[i]);
            }
        }
        if (snowflake != 0) {
            game->players.insert(std::pair<dpp::snowflake, Player>(snowflake,
                                                                   Player(snowflake, sucCount, failCount,
                                                                          highestCount)));
        } else {
            std::cout << "no snowflake in init of players bad data in data base";
        }
        return 0;
    }

    //calback for loading server data
    int CountingGame::setServerValues(void *countGamePtr, int entries, char **value, char **colName) {
        uint64_t channelId = 0;
        uint64_t currentCount = 0;
        uint64_t lastPlayerId = 0;

        for (int i = 0; i < entries; i++) {
            if (strcmp(colName[i], "CHANNEL_ID") == 0) {
                channelId = std::stoll(value[i]);
            } else if (strcmp(colName[i], "CURRENT_COUNT") == 0) {
                currentCount = std::stoll(value[i]);
            } else if (strcmp(colName[i], "LAST_PLAYER") == 0) {
                lastPlayerId = std::stoll(value[i]);
            }
        }
        CountingGame *game = (CountingGame *) countGamePtr;
        game->currentCount = currentCount;
        auto pair = game->players.find(channelId);
        if (pair != game->players.end()) {
            game->lastPlayer = &pair->second;
        }
        game->channelID = channelId;
        return 0;
    }

    //save a player data back to the db
    void CountingGame::savePlayer(const Player &player) {
        std::stringstream ss;
        ss << "UPDATE PLAYER" << "\n";
        ss << "SET TOTAL_CORRECT = " << player.getCorectCount();
        ss << " , TOTAL_FAILED = " << player.getFailedCount();
        ss << " , HIGHEST_COUNT = " << player.getHighestCount() << "\n";
        ss << "WHERE SNOWFLAKE = " << player.userId << ";";

        char *errmsg;
        int ec = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &errmsg);
        if (ec != SQLITE_OK) {
            std::cout << "failed to save player " << errmsg << "\n";
            sqlite3_free(errmsg);
        }
    }

    //save the game data back to the db
    void CountingGame::saveGame() {
        std::stringstream ss;
        ss << "UPDATE GAME" << "\n";
        ss << "SET CHANNEL_ID = " << channelID;
        ss << " , CURRENT_COUNT = " << currentCount;
        if (lastPlayer != nullptr) {
            ss << " , LAST_PLAYER = " << lastPlayer->userId;
        } else {
            ss << " , LAST_PLAYER = " << 0;
        }
        ss << ";";
        char *errmsg;
        int ec = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &errmsg);
        if (ec != SQLITE_OK) {
            std::cout << "failed to save game " << errmsg << "\n";
            sqlite3_free(errmsg);
        }
    }

    //add new player entry to the db and hashtable
    Player &CountingGame::addPlayer(dpp::snowflake id) {
        auto value = players.insert({id, Player{id}});
        std::stringstream ss;
        ss << "INSERT INTO PLAYER VALUES (" << id << "," << 0 << "," << 0 << "," << 0 << ");";

        char *errmsg;
        int ec = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &errmsg);
        if (ec != SQLITE_OK) {
            std::cout << "failed to add player to DB " << errmsg << "\n";
            sqlite3_free(errmsg);
        }
        return value.first->second;
    }

    //calculate and check a message and update game data
    void Bot::CountingGame::count(dpp::cluster &bot, const dpp::message_create_t &message) {
        if (message.msg->channel_id == channelID) {
            if (!shouldCount(message.msg->content)) {
                return;
            }
            const dpp::snowflake author = message.msg->author->id;
            if (currentCount != resetCount && lastPlayer != nullptr && author == lastPlayer->userId) {
                reply(CountingGame::Type::SAME_USER, bot, message, 0);
                auto keySet = players.find(author);
                if (keySet != players.end()) {
                    addCountToPlayer(keySet->second, false);
                    saveGame();
                }
                return;
            }
            std::string_view view(message.msg->content);
            auto RPNList = StringCalculator::convertStringToRPNList(view);
            double value = StringCalculator::calculateFromRPNList(RPNList);
            if (std::isnan(value)) return;
            value = std::floor(value);
            bool isCorrect = value == ++currentCount;
            auto keySet = players.find(author);
            if (keySet != players.end()) {
                reply(addCountToPlayer(keySet->second, isCorrect), bot, message, value);
                saveGame();
                return;
            } else {
                auto player = addPlayer(author);
                reply(addCountToPlayer(player, isCorrect), bot, message, value);
                saveGame();
                return;
            }
        }
    }

    //reply with message depending on type
    void CountingGame::reply(const Type type, dpp::cluster &bot, const dpp::message_create_t &message, double value) {
        int64_t cast = value;
        switch (type) {
            case Bot::CountingGame::Type::CORRECT:
                bot.message_add_reaction(message.msg->id, message.msg->channel_id, "✅");
                break;
            case Bot::CountingGame::Type::INCORRECT:
                bot.message_add_reaction(message.msg->id, message.msg->channel_id, "❌");
                bot.message_create(dpp::message(
                        message.msg->channel_id,
                        "<@" + std::to_string(message.msg->author->id) +
                        "> Cant count. Gave Value " + std::to_string(cast) + ". Count reset to 1"
                ));
                currentCount = resetCount;
                break;
            case Bot::CountingGame::Type::SAME_USER:
                bot.message_add_reaction(message.msg->id, message.msg->channel_id, "❌");
                bot.message_create(dpp::message(
                        message.msg->channel_id,
                        "<@" + std::to_string(message.msg->author->id) +
                        "> You already counted. Count reset to 1"
                ));
                currentCount = 0;
                break;
        }
    }

    //handle player data
    CountingGame::Type CountingGame::addCountToPlayer(Player &player, bool correct) {
        if (correct) {
            player.incrementCorrectCount();
            player.checkAndSetHighestCount(currentCount);
            lastPlayer = &player;
            savePlayer(player);
            return CORRECT;
        } else {
            player.incrementFailedCount();
            savePlayer(player);
            return INCORRECT;
        }
    }

    //a check if the message should be calculated
    //TODO allow RPN and and number prefix
    bool CountingGame::shouldCount(const std::string_view &input) {
        const char *index = input.data();
        const char *end = input.data() + input.size();
        while (index < end) {
            if (*index == '<') return false;
            if (*index == ' ') {
                index++;
                continue;
            }
            if (*index == '(') return true;
            char32_t unicode;
            index = StringCalculator::getUnicode(index, unicode);
            int number = StringCalculator::getNumberFromUnicode(unicode);
            if (number != -1) return true;
            auto op = StringCalculator::getOperator(unicode);
            return op != nullptr;
        }
        return false;
    }

    dpp::snowflake Bot::CountingGame::getCountChannel() const {
        return channelID;
    }

    void Bot::CountingGame::setCountChannel(dpp::snowflake channelID) {
        this->channelID = channelID;
        saveGame();
    }

    //get ranking of players
    //TODO replace with db call
    std::vector<const Player *> Bot::CountingGame::getRankedPlayers(uint32_t amount, uint32_t startPos) const {
        std::vector<const std::pair<const dpp::snowflake, Player> *> playerList(players.size());
        int i = 0;
        for (auto &player: players) {
            playerList[i++] = &player;
        }
        std::sort(playerList.begin(), playerList.end(),
                  [](const std::pair<const dpp::snowflake, Player> *p1,
                     const std::pair<const dpp::snowflake, Player> *p2) {
                      return p1->second.getHighestCount() >= p2->second.getHighestCount();
                  });

        if (startPos + amount > playerList.size()) {
            uint64_t toPushBack = amount + startPos - playerList.size();
            if (toPushBack > startPos) {
                startPos = 0;
                amount = playerList.size();
            }
        }

        std::vector<const Player *> returnList(amount);
        for (i = 0; i < amount; i++) {
            returnList[i] = &playerList[i + startPos]->second;
        }
        return returnList;
    }

    bool CountingGame::onInteraction(const dpp::interaction_create_t &interaction) {
        if (interaction.command.type == dpp::it_application_command) {
            dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
            auto result = interactions.find(cmd_data.id);
            if (result != interactions.end()) {
                result->second(interaction);
                return true;
            }
        }
        return false;
    }

    void CountingGame::addCommands(dpp::cluster &bot, Settings &settings) {
        {
            dpp::slashcommand setChannel;
            std::string nameSetChannel = "set_counting_channel";
            setChannel.set_name(nameSetChannel);
            setChannel.set_description("set the channel in wich the counting happens");
            setChannel.set_type(dpp::ctxm_chat_input);
            setChannel.set_application_id(bot.me.id);
            setChannel.disable_default_permissions();
            for (auto &command: settings.getCommandPermissions(nameSetChannel)) {
                setChannel.add_permission(command);
            }
            setChannel.add_option(
                    dpp::command_option(
                            dpp::co_channel, "channel", "channel where counting happens", true
                    ));
            registerCommand(bot, settings, setChannel, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                auto value = std::get<dpp::snowflake>(cmd_data.options[0].value);
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                          .set_type(dpp::mt_reply)
                                          .set_flags(dpp::m_ephemeral)
                                          .set_content(
                                                  "<#" + std::to_string(value) + "> has been set as counting channel")
                );
                setCountChannel(value);
            });
        }

        {
            dpp::slashcommand testNumber;
            std::string nameTestNumber = "test_number";
            testNumber.set_name(nameTestNumber);
            testNumber.set_type(dpp::ctxm_chat_input);
            testNumber.set_description("tests number with stack trace");
            testNumber.set_application_id(bot.me.id);
            for (auto &command: settings.getCommandPermissions(nameTestNumber)) {
                testNumber.add_permission(command);
            }
            testNumber.add_option(
                    dpp::command_option(
                            dpp::co_string, "calculation", "calculation", true
                    ));
            registerCommand(bot, settings, testNumber, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                auto input = std::get<std::string>(cmd_data.options[0].value);
                std::string_view view(input);
                auto list = Bot::StringCalculator::convertStringToRPNList(view);
                double value = Bot::StringCalculator::calculateFromRPNList(list);
                std::stringstream ss;
                ss << "value = " << value << "\n";
                ss << "RPN stacktrace" << "\n";
                for (auto &count: list) {
                    if (count->isOperator()) {
                        Operator *op = (Operator *) count.get();
                        ss << Bot::StringCalculator::unicodeToString(op->unicode) << "\n";
                    } else {
                        Number *num = (Number *) count.get();
                        ss << num->value << "\n";
                    }
                }
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                          .set_type(dpp::mt_reply)
                                          .set_flags(dpp::m_ephemeral)
                                          .set_content(ss.str())
                );
            });
        }

        {
            dpp::slashcommand getStats;
            std::string nameGetStats = "get_stats";
            getStats.set_name(nameGetStats);
            getStats.set_description("get top 10 player stats from pos");
            getStats.set_type(dpp::ctxm_chat_input);
            getStats.set_application_id(bot.me.id);
            for (auto &command: settings.getCommandPermissions(nameGetStats)) {
                getStats.add_permission(command);
            }
            getStats.add_option(
                    dpp::command_option(
                            dpp::co_integer, "start_pos", "starts pos of listing"
                    ));
            registerCommand(bot, settings, getStats, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                uint64_t startPos = 0;
                if (!cmd_data.options.empty()) {
                    startPos = std::get<int64_t>(cmd_data.options[0].value);
                }
                auto players = getRankedPlayers(10, startPos);
                std::stringstream ss;
                ss << "Ranking from pos " << startPos << "\n";
                int i = 0;
                for (auto &player: players) {
                    ss << "#" << i++ << " <@" << player->userId << ">, " << player->getHighestCount() << "\n";
                }
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                          .set_type(dpp::mt_reply)
                                          .set_flags(dpp::m_ephemeral)
                                          .set_content(ss.str())
                );
            });
        }

        {
            dpp::slashcommand getPlayerStats;
            std::string nameGetPlayerStats = "get_user_stats";
            getPlayerStats.set_name(nameGetPlayerStats);
            getPlayerStats.set_description("get stats of a user");
            getPlayerStats.set_type(dpp::ctxm_chat_input);
            getPlayerStats.set_application_id(bot.me.id);
            for (auto &command: settings.getCommandPermissions(nameGetPlayerStats)) {
                getPlayerStats.add_permission(command);
            }
            getPlayerStats.add_option(
                    dpp::command_option(
                            dpp::co_user, "user", "user to get stts from"
                    ));
            registerCommand(bot, settings, getPlayerStats, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                dpp::snowflake user_id;
                if (!cmd_data.options.empty()) {
                    user_id = std::get<dpp::snowflake>(cmd_data.options[0].value);
                } else {
                    user_id = interaction.command.usr.id;
                }
                //350016920264638485
                auto userPair = players.find(user_id);
                if (userPair != players.end()) {
                    auto &user = userPair->second;
                    std::stringstream ss;
                    ss << "<@" << user_id << "> Stats.\n";
                    ss << "Highest Count = " << user.getHighestCount() << "\n";
                    ss << "Success Rate = " << std::setprecision(3) << user.getSuccessRate() << "\n";
                    ss << "Total Count = " << user.getTotalCount() << "\n";
                    ss << "Total Correct = " << user.getCorectCount() << "\n";
                    ss << "Total Failed = " << user.getFailedCount() << "\n";
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_flags(dpp::m_ephemeral)
                                              .set_content(ss.str())
                    );
                } else {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_flags(dpp::m_ephemeral)
                                              .set_content("User Not Found")
                    );
                }
            });
        }
    }

    void CountingGame::registerCommand(dpp::cluster &bot, Settings &settings, dpp::slashcommand &command,
                                       std::function<void(
                                               const dpp::interaction_create_t &interaction)> interactionCallBack) {
        bot.guild_command_create(command, settings.getServerId(), [&bot, command, this, interactionCallBack](
                const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
                std::cout << callback.get_error().message << "\n";
            }
            dpp::slashcommand cmd_data = std::get<dpp::slashcommand>(callback.value);
            uint64_t id = cmd_data.id;
            auto returnValue = this->commands.insert(std::pair<uint64_t, dpp::slashcommand>(id, command));
            if (returnValue.second) {
                returnValue.first->second.id = id;
                this->interactions.insert(
                        std::pair<uint64_t, std::function<void(const dpp::interaction_create_t &interaction)>>(
                                id,
                                interactionCallBack
                        ));
                std::cout << "command " << returnValue.first->second.name << "ready";
            } else {
                std::cout << "ERROR failed to register command" << "\n";
            }
        });
    }


}