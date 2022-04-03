//
// Created by vanda on 04/10/2021.
//

#include "../include/counting_game.hpp"
#include "../include/string_calculator.hpp"
#include "../include/app.hpp"

#include <sstream>
#include <string>
#include <cmath>
#include <iomanip>


namespace Bot {
    Bot::CountingGame::CountingGame(App *app, int id) :
            channelID{0}, currentCount{0}, lastPlayer{nullptr}, id{id} {
        auto dataBase = initSettings(app);

        StringCalculator::init(app->settings->getUnicodeTranslationLocation());

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

    std::string CountingGame::initSettings(App *app) {
        auto &doc = app->settings->getDocument();
        if (!doc.HasMember("counting")) {
            rapidjson::Value counting;
            counting.SetObject();
            doc.AddMember("counting", counting, doc.GetAllocator());
        }
        auto &count = doc["counting"];

        if (!count.HasMember("count_game_db_location")) {
            rapidjson::Value db;
            db.SetString("CountData.db");
            count.AddMember("count_game_db_location", db, doc.GetAllocator());
            std::cout
                    << "no value for counting DB location found in settings. Putting it in default location. ~/CountData.db\n";
            app->settings->save();
        }
        return count["count_game_db_location"].GetString();
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
                sqlite3_free(msgError);
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
        addExtraTables();
    }

    //sqlite has no way to check if a table exists
    //so im just going to always run these commnads and just ignore the error
    void CountingGame::addExtraTables() {
        char *err;
        int ec = sqlite3_exec(db, "ALTER TABLE GAME ADD FAIL_ROLL_ID INT;", 0, 0, &err);
        if (SQLITE_OK != ec) {
            sqlite3_free(err);
        }
        ec = sqlite3_exec(db, "ALTER TABLE GAME ADD BOT_CHANNEL INT;", 0, 0, &err);
        if (SQLITE_OK != ec) {
            sqlite3_free(err);
        }
        ec = sqlite3_exec(db, "ALTER TABLE PLAYER ADD SAVES INT;", 0, 0, &err);
        if (SQLITE_OK != ec) {
            sqlite3_free(err);
        }
    }

    //callback for loading player data
    int CountingGame::populateHashSet(void *data, int argc, char **value, char **colName) {
        CountingGame *game = (CountingGame *) data;
        uint64_t snowflake = 0;
        uint64_t sucCount = 0;
        uint64_t failCount = 0;
        uint64_t highestCount = 0;
        uint32_t saves = 0;
        for (int i = 0; i < argc; i++) {
            if (value[i] == NULL) continue;
            if (strcmp(colName[i], "SNOWFLAKE") == 0) {
                snowflake = std::stoull(value[i]);
            } else if (strcmp(colName[i], "TOTAL_CORRECT") == 0) {
                sucCount = std::stoull(value[i]);
            } else if (strcmp(colName[i], "TOTAL_FAILED") == 0) {
                failCount = std::stoull(value[i]);
            } else if (strcmp(colName[i], "HIGHEST_COUNT") == 0) {
                highestCount = std::stoull(value[i]);
            } else if (strcmp(colName[i], "SAVES") == 0) {
                saves = std::stoul(value[i]);
            }
        }
        if (snowflake != 0) {
            game->players.insert(std::pair<dpp::snowflake, Player>(snowflake,
                                                                   Player(snowflake, sucCount, failCount,
                                                                          highestCount, saves)));
            if (game->highestCount < highestCount) {
                game->highestCount = highestCount;
            }
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
        uint64_t rollID = 0;

        for (int i = 0; i < entries; i++) {
            if (value[i] == NULL) continue;
            if (strcmp(colName[i], "CHANNEL_ID") == 0) {
                channelId = std::stoll(value[i]);
            } else if (strcmp(colName[i], "CURRENT_COUNT") == 0) {
                currentCount = std::stoll(value[i]);
            } else if (strcmp(colName[i], "LAST_PLAYER") == 0) {
                lastPlayerId = std::stoll(value[i]);
            } else if (strcmp(colName[i], "FAIL_ROLL_ID") == 0) {
                rollID = std::stoll(value[i]);
            }
        }
        CountingGame *game = (CountingGame *) countGamePtr;
        game->currentCount = currentCount;
        auto pair = game->players.find(lastPlayerId);
        if (pair != game->players.end()) {
            game->lastPlayer = &pair->second;
        }
        game->channelID = channelId;
        game->failRollID = rollID;
        return 0;
    }

    //save a player data back to the db
    void CountingGame::savePlayer(const Player &player) {
        std::stringstream ss;
        ss << "UPDATE PLAYER" << "\n";
        ss << "SET TOTAL_CORRECT = " << player.getCorectCount();
        ss << " , TOTAL_FAILED = " << player.getFailedCount();
        ss << " , HIGHEST_COUNT = " << player.getHighestCount() << "\n";
        ss << " , SAVES = " << player.getSaves() << "\n";
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
        ss << " , FAIL_ROLL_ID = " << failRollID;
        ss << " , BOT_CHANNEL = " << botSpam;
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
        ss << "INSERT INTO PLAYER VALUES (" << id << "," << 0 << "," << 0 << "," << 0 << ","<< 0 <<");";

        char *errmsg;
        int ec = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &errmsg);
        if (ec != SQLITE_OK) {
            std::cout << "failed to add player to DB " << errmsg << "\n";
            sqlite3_free(errmsg);
        }
        return value.first->second;
    }

    //calculate and check a message and update game data
    void Bot::CountingGame::count(App *app, const dpp::message_create_t &message) {
        if (message.msg->channel_id == channelID) {
            if (!shouldCount(message.msg->content)) {
                return;
            }
            const dpp::snowflake author = message.msg->author->id;
            if (currentCount != resetCount && lastPlayer != nullptr && author == lastPlayer->userId) {
                if (lastPlayer->getSaves() > 0) {
                    lastPlayer->setSaves(lastPlayer->getSaves() - 1);
                    reply(SAVED, app, message, lastPlayer->getSaves());
                    savePlayer(*lastPlayer);
                    return;
                }
                reply(CountingGame::Type::SAME_USER, app, message, 0);
                auto keySet = players.find(author);
                if (keySet != players.end()) {
                    addCountToPlayer(app, keySet->second, false);
                    saveGame();
                }
                return;
            }
            std::string_view view(message.msg->content);
            auto RPNList = StringCalculator::convertStringToRPNList(view);
            if (RPNList.empty()) return;
            double value = StringCalculator::calculateFromRPNList(RPNList);
            if (std::isnan(value)) return;
            if (std::isinf(value)) return;
            value = std::floor(value);
            bool isCorrect = StringCalculator::floor(value) == ++currentCount;
            if (isCorrect) lastMessage = message.msg->id;
            auto keySet = players.find(author);
            if (isCorrect && highestCount < currentCount) {
                highestCount = currentCount;
            }
            if (keySet != players.end()) {
                auto type = addCountToPlayer(app, keySet->second, isCorrect);
                reply(type, app, message, value);
                if (type == SAVED) currentCount--;
                saveGame();
                return;
            } else {
                auto player = addPlayer(author);
                auto type = addCountToPlayer(app, player, isCorrect);
                reply(type, app, message, value);
                if (type == SAVED) currentCount--;
                saveGame();
                return;
            }
        }
    }

    //reply with message depending on type
    void CountingGame::reply(const Type type, App *app, const dpp::message_create_t &message, double value) {
        int64_t cast = value;
        switch (type) {
            case Bot::CountingGame::Type::CORRECT:
                if (currentCount == highestCount) {
                    app->bot->message_add_reaction(message.msg->id, message.msg->channel_id, "☑");
                } else {
                    app->bot->message_add_reaction(message.msg->id, message.msg->channel_id, "✅");
                }
                break;
            case Bot::CountingGame::Type::INCORRECT:
                app->bot->message_add_reaction(message.msg->id, message.msg->channel_id, "❌");
                app->bot->message_create(dpp::message(
                        message.msg->channel_id,
                        "<@" + std::to_string(message.msg->author->id) +
                        "> Cant count. Gave Value " + std::to_string(cast) + ". Count reset to 1"
                ));
                currentCount = resetCount;
                break;
            case Bot::CountingGame::Type::SAME_USER:
                app->bot->message_add_reaction(message.msg->id, message.msg->channel_id, "❌");
                app->bot->message_create(dpp::message(
                        message.msg->channel_id,
                        "<@" + std::to_string(message.msg->author->id) +
                        "> You already counted. Count reset to 1"
                ));
                currentCount = 0;
                break;
            case Bot::CountingGame::Type::SAVED:
                std::stringstream ss;
                app->bot->message_create(dpp::message(
                        message.msg->channel_id,
                        "<@" + std::to_string(message.msg->author->id) + "> almost lost but got saved. Saves left " +
                        std::to_string((uint64_t)value)
                ));
                break;
        }
    }

    //handle player data
    CountingGame::Type CountingGame::addCountToPlayer(App *app, Player &player, bool correct) {
        if (correct) {
            player.incrementCorrectCount();
            player.checkAndSetHighestCount(currentCount);
            lastPlayer = &player;
            savePlayer(player);
            return CORRECT;
        } else {
            if (player.getSaves() > 0) {
                player.setSaves(player.getSaves() - 1);
                savePlayer(player);
                return SAVED;
            }
            player.incrementFailedCount();
            savePlayer(player);
            if (failRollID != 0) {
                app->bot->guild_member_add_role(app->settings->getServerId(), player.userId, failRollID);
            }
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
            char32_t unicode;
            StringCalculator::getUnicode(index, unicode);
            if (StringCalculator::isUnicodeUsed(unicode)) return true;
            auto funcStr = std::string(StringCalculator::getFunctionString(&index,end));
            return StringCalculator::hasFunction(funcStr) || StringCalculator::hasConst(funcStr);
        }
        return false;
    }

    void CountingGame::onMessageDelete(App *app, const dpp::message_delete_t &event) {
        auto message =  event.deleted;
        if (message->channel_id == channelID && message->id == lastMessage) {
            std::stringstream ss;
            ss<<"Last count message deleted. Current count is ";
            ss<<currentCount<<".";
            app->bot->message_create(dpp::message(channelID,ss.str()));
        }
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

    void CountingGame::addSettings(dpp::slashcommand &baseCommand, App *app) {
        dpp::command_option command(dpp::co_sub_command_group, "counting", "counting settings");
        {
            dpp::command_option setChannel(dpp::co_sub_command, "set_counting_channel",
                                           "set the channel in wich the counting happens");
            setChannel.add_option(
                    dpp::command_option(
                            dpp::co_channel, "channel", "channel where counting happens", true
                    ));
            command.add_option(setChannel);
            app->registerSetting(baseCommand, command, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);

                auto value = std::get<dpp::snowflake>(cmd_data.options[0].options[0].options[0].value);
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                          .set_type(dpp::mt_reply)
                                          .set_flags(dpp::m_ephemeral)
                                          .set_content(
                                                  "<#" + std::to_string(value) + "> has been set as counting channel")
                );
                setCountChannel(value);
            }, 0);
        }

        {
            dpp::command_option setFailRoll(dpp::co_sub_command, "set_fail_roll",
                                            "set which roll it should give the player if they count wrong");
            setFailRoll.add_option(
                    dpp::command_option(
                            dpp::command_option(dpp::co_role, "fail_roll", "leave option out to not give roll")
                    ));
            command.add_option(setFailRoll);
            app->registerSetting(baseCommand, command, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                auto &data = cmd_data.options[0].options[0];
                std::stringstream ss;
                if (!data.options.empty()) {
                    auto value = std::get<dpp::snowflake>(data.options[0].value);
                    failRollID = value;
                    ss << "Fail Roll has been set to <@&" << failRollID << ">";
                } else {
                    failRollID = 0;
                    ss << "Fail Roll has been reset and no roll will be given on fail";
                }
                saveGame();
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                          .set_type(dpp::mt_reply)
                                          .set_flags(dpp::m_ephemeral)
                                          .set_content(ss.str()));
            }, 1);
        }

        {
            dpp::command_option setFailRoll(dpp::co_sub_command, "set_bot_channel",
                                            "set which channel is the bot spam channel");
            setFailRoll.add_option(
                    dpp::command_option(
                            dpp::command_option(dpp::co_channel, "bot_channel", "if empty no channel is the bot spam channel")
                    ));
            command.add_option(setFailRoll);
            app->registerSetting(baseCommand, command, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                auto &data = cmd_data.options[0].options[0];
                std::stringstream ss;
                if (!data.options.empty()) {
                    auto value = std::get<dpp::snowflake>(data.options[0].value);
                    botSpam = value;
                    ss << "bot spam channel set to <#" << botSpam << ">";
                } else {
                    botSpam = 0;
                    ss << "no channel for bot spam";
                }
                saveGame();
                interaction.reply(dpp::ir_channel_message_with_source,
                                  dpp::message()
                                          .set_type(dpp::mt_reply)
                                          .set_flags(dpp::m_ephemeral)
                                          .set_content(ss.str()));
            }, 2);
        }

        {
            dpp::command_option setSaves(dpp::co_sub_command, "set_saves", "set the save count of user");
            dpp::command_option player(dpp::co_user, "user", "wich user the saves is set");
            dpp::command_option amount(dpp::co_integer, "amount", "to wich amount the saves should be set", true);
            setSaves.add_option(amount);
            setSaves.add_option(player);
            command.add_option(setSaves);
            app->registerSetting(baseCommand, command, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                auto &data = cmd_data.options[0].options[0];
                dpp::snowflake userID;
                uint64_t amount;
                if (data.options.size()==1) {
                    amount = std::get<int64_t>(data.options[0].value);
                    userID = interaction.command.usr.id;
                } else {
                    if (data.options[0].name == "amount") {
                        amount = std::get<int64_t>(data.options[0].value);
                        userID = std::get<dpp::snowflake>(data.options[1].value);
                    } else {
                        userID = std::get<dpp::snowflake>(data.options[0].value);
                        amount = std::get<int64_t>(data.options[1].value);
                    }
                }
                if (amount < 0) amount = 0;
                std::stringstream ss;
                auto player = players.find(userID);
                if (player != players.end()) {
                    player->second.setSaves(amount);
                    ss<<"User <@"<<userID<<"> Saves has been set to "<<amount;
                } else {
                    ss<<"User <@"<<userID<<"> Was not found in the player list";
                }
                interaction.reply(dpp::ir_channel_message_with_source,dpp::message()
                .set_type(dpp::mt_reply).set_flags(dpp::m_ephemeral)
                .set_content(ss.str()));
            }, 3);
        }

        baseCommand.add_option(command);
    }

    void CountingGame::addCommands(dpp::cluster &bot, Settings &settings, App *app) {
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
            app->registerCommand(bot, settings, testNumber, [this](const dpp::interaction_create_t &interaction) {
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
                        ss << op->name << "\n";
                    } else {
                        Number *num = (Number *) count.get();
                        ss << num->value << "\n";
                    }
                }
                if (interaction.command.channel_id == botSpam) {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_content(ss.str())
                    );
                } else {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_flags(dpp::m_ephemeral)
                                              .set_content(ss.str())
                    );
                }
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
            app->registerCommand(bot, settings, getStats, [this](const dpp::interaction_create_t &interaction) {
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
                if (interaction.command.channel_id == botSpam) {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_content(ss.str())
                    );
                } else {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_flags(dpp::m_ephemeral)
                                              .set_content(ss.str())
                    );
                }
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
                            dpp::co_user, "user", "user to get stats from"
                    ));
            app->registerCommand(bot, settings, getPlayerStats, [this](const dpp::interaction_create_t &interaction) {
                dpp::command_interaction cmd_data = std::get<dpp::command_interaction>(interaction.command.data);
                dpp::snowflake user_id;
                if (!cmd_data.options.empty()) {
                    user_id = std::get<dpp::snowflake>(cmd_data.options[0].value);
                } else {
                    user_id = interaction.command.usr.id;
                }
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
                    if (interaction.command.channel_id == botSpam) {
                        interaction.reply(dpp::ir_channel_message_with_source,
                                          dpp::message()
                                                  .set_type(dpp::mt_reply)
                                                  .set_content(ss.str())
                        );
                    } else {
                        interaction.reply(dpp::ir_channel_message_with_source,
                                          dpp::message()
                                                  .set_type(dpp::mt_reply)
                                                  .set_flags(dpp::m_ephemeral)
                                                  .set_content(ss.str())
                        );
                    }
                } else {
                    if (interaction.command.channel_id == botSpam) {
                        interaction.reply(dpp::ir_channel_message_with_source,
                                          dpp::message()
                                                  .set_type(dpp::mt_reply)
                                                  .set_content("User Not Found")
                        );
                    } else {
                        interaction.reply(dpp::ir_channel_message_with_source,
                                          dpp::message()
                                                  .set_type(dpp::mt_reply)
                                                  .set_flags(dpp::m_ephemeral)
                                                  .set_content("User Not Found")
                        );
                    }
                }
            });
        }
        {
            dpp::slashcommand command;
            std::string name = "get_server_stats";
            command.set_name(name);
            command.set_description("get general game stats");
            command.set_type(dpp::ctxm_chat_input);
            command.set_application_id(bot.me.id);
            for (auto &com: settings.getCommandPermissions(name)) {
                command.add_permission(com);
            }
            app->registerCommand(bot, settings, command, [this](const dpp::interaction_create_t &interaction) {
                std::stringstream ss;
                ss << "Server Stats\n";
                ss << "Current Count = " << currentCount << "\n";
                if (lastPlayer != nullptr) {
                    ss << "Last User <@" << lastPlayer->userId << ">\n";
                } else {
                    ss << "No Last User\n";
                }
                ss << "Highest Count = " << highestCount << "\n";
                if (interaction.command.channel_id == botSpam) {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_content(ss.str())
                    );
                } else {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_flags(dpp::m_ephemeral)
                                              .set_content(ss.str())
                    );
                }
            });
        }
        {
            dpp::slashcommand command;
            std::string name = "syntax";
            command.set_name(name);
            command.set_description("print all valid syntax");
            command.set_type(dpp::ctxm_chat_input);
            command.set_application_id(bot.me.id);
            for (auto &com: settings.getCommandPermissions(name)) {
                command.add_permission(com);
            }
            app->registerCommand(bot, settings, command, [this](const dpp::interaction_create_t &interaction) {
                std::stringstream ss;
                ss<<"valid digits\n";
                for (auto &value : StringCalculator::getNumberMap()) {
                    ss<<StringCalculator::unicodeToString(value.first)<<" : "<<value.second<<"\n";
                }
                ss<<"\nvalid operators\n";
                for (auto &value: StringCalculator::getOperatorMap()) {
                    ss<<StringCalculator::unicodeToString(value.first)<<"\n";
                }
                ss<<"\nvalid constants\n";
                for (auto &value: StringCalculator::getConstMap()) {
                    ss<<value.first<<" : "<<value.second<<"\n";
                }
                ss<<"\nvalid functions\n";
                for (auto &value: StringCalculator::getFunctionMap()) {
                    ss<<value.first<<"\n";
                }
                if (interaction.command.channel_id == botSpam) {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_content(ss.str())
                    );
                } else {
                    interaction.reply(dpp::ir_channel_message_with_source,
                                      dpp::message()
                                              .set_type(dpp::mt_reply)
                                              .set_flags(dpp::m_ephemeral)
                                              .set_content(ss.str())
                    );
                }
            });
        }
    }


}