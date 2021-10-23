//
// Created by vanda on 04/10/2021.
//

#include "../include/counting_game.hpp"
#include "../include/string_calculator.hpp"

#include <sstream>
#include <string>
#include <cmath>


namespace Bot {
    Bot::CountingGame::CountingGame(std::string dataBase, int id) :
            channelID{0}, currentCount{0}, lastPlayer{nullptr}, id{id} {
        int error = sqlite3_open(dataBase.c_str(),&db);
        if (error) {
            std::cout<<"failed to open dataBase"<<sqlite3_errmsg(db)<<"\n";
            sqlite3_close(db);
            throw std::runtime_error("failed to open dataBase");
        }
        initDataBase(id);
    }

    CountingGame::~CountingGame() {
        sqlite3_close(db);
    }

    void CountingGame::initDataBase(int id) {
        char * msgError;
        int error = sqlite3_exec(db,
                                 "CREATE TABLE PLAYER("
                                 "SNOWFLAKE INT PRIMARY KEY NOT NULL,"
                                 "TOTAL_CORRECT INT NOT NULL,"
                                 "TOTAL_FAILED INT NOT NULL,"
                                 "HIGHEST_COUNT INT NOT NULL);",
                                 NULL,0,&msgError);
        if (error != SQLITE_OK) { //if it erros that should mean the table already exists so then pull data from db to memory
            sqlite3_free(msgError);
            //load player data
            error =  sqlite3_exec(db,"SELECT * FROM PLAYER;",CountingGame::populateHashSet,this,&msgError);
            if (error != SQLITE_OK) {
                std::cout<<"on players init"<<msgError<<"\n";
                sqlite3_free(msgError);
                throw std::runtime_error("failed to load player data");
            }
            std::stringstream ss;
            //load game data
            ss<<"SELECT * FROM GAME WHERE ID = "<<id<<";";
            error =  sqlite3_exec(db,ss.str().c_str(),CountingGame::setServerValues,this,&msgError);
            if (error != SQLITE_OK) {
                std::cout<<"on game init"<<msgError<<"\n";
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
                         NULL,NULL,&msgError);
            if (ec != SQLITE_OK) {
                std::cout<<"crated player table but faild to create server database could be mallformed"<<"\n"
                <<msgError<<"\n";
                throw std::runtime_error("error on database init");
            } else {
                std::stringstream ss;
                ss<<"INSERT INTO GAME VALUES("<<id<<","<<0<<","<<0<<","<<0<<");";
                ec = sqlite3_exec(db,ss.str().c_str(),NULL,NULL,&msgError);
                if (ec != SQLITE_OK) {
                    std::cout<<"failed to insert game in to db "<<msgError<<"\n";
                    sqlite3_free(msgError);
                    throw std::runtime_error("failed to insert game data in to DB");
                }
            }
        }
    }

    //callback for loading player data
    int CountingGame::populateHashSet(void *data, int argc, char **value, char **colName) {
        CountingGame *game = (CountingGame*)data;
        uint64_t snowflake = 0;
        uint64_t sucCount = 0;
        uint64_t failCount = 0;
        uint64_t highestCount = 0;
        for (int i = 0;i<argc;i++) {
            if (strcmp(colName[i],"SNOWFLAKE") == 0) {
                snowflake = std::stoll(value[i]);
            } else if (strcmp(colName[i],"TOTAL_CORRECT") == 0) {
                sucCount = std::stoll(value[i]);
            } else if (strcmp(colName[i],"TOTAL_FAILED") == 0) {
                failCount = std::stoll(value[i]);
            }else if (strcmp(colName[i],"HIGHEST_COUNT") == 0) {
                highestCount = std::stoll(value[i]);
            }
        }
        if (snowflake != 0) {
            game->players.insert(std::pair<dpp::snowflake,Player>(snowflake,
                                                                  Player(snowflake,sucCount,failCount,highestCount)));
        } else {
            std::cout<<"no snowflake in init of players bad data in data base";
        }
        return 0;
    }

    //calback for loading server data
    int CountingGame::setServerValues(void *countGamePtr, int entries, char **value, char **colName) {
        uint64_t channelId = 0;
        uint64_t currentCount = 0;
        uint64_t lastPlayerId = 0;

        for (int i = 0;i<entries;i++) {
            if (strcmp(colName[i],"CHANNEL_ID") == 0) {
                channelId = std::stoll(value[i]);
            } else if (strcmp(colName[i],"CURRENT_COUNT") == 0) {
                currentCount = std::stoll(value[i]);
            } else if (strcmp(colName[i],"LAST_PLAYER") == 0) {
                lastPlayerId = std::stoll(value[i]);
            }
        }
        CountingGame *game = (CountingGame*) countGamePtr;
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
        ss<<"UPDATE PLAYER"<<"\n";
        ss<<"SET TOTAL_CORRECT = "<<player.getCorectCount();
        ss<<" , TOTAL_FAILED = "<<player.getFailedCount();
        ss<<" , HIGHEST_COUNT = "<<player.getHighestCount()<<"\n";
        ss<<"WHERE SNOWFLAKE = "<<player.userId<<";";

        char * errmsg;
        int ec = sqlite3_exec(db,ss.str().c_str(),NULL,NULL,&errmsg);
        if (ec != SQLITE_OK) {
            std::cout<<"failed to save player "<<errmsg<<"\n";
            sqlite3_free(errmsg);
        }
    }

    //save the game data back to the db
    void CountingGame::saveGame() {
        std::stringstream ss;
        ss<<"UPDATE GAME"<<"\n";
        ss<<"SET CHANNEL_ID = "<<channelID;
        ss<<" , CURRENT_COUNT = "<<currentCount;
        if (lastPlayer != nullptr) {
            ss<<" , LAST_PLAYER = "<<lastPlayer->userId;
        } else {
            ss<<" , LAST_PLAYER = "<<0;
        }
        ss<<";";
        char * errmsg;
        int ec = sqlite3_exec(db,ss.str().c_str(),NULL,NULL,&errmsg);
        if (ec != SQLITE_OK) {
            std::cout<<"failed to save game "<<errmsg<<"\n";
            sqlite3_free(errmsg);
        }
    }

    //add new player entry to the db and hashtable
    Player &CountingGame::addPlayer(dpp::snowflake id) {
        auto value = players.insert({id, Player{id}});
        std::stringstream ss;
        ss<<"INSERT INTO PLAYER VALUES ("<<id<<","<<0<<","<<0<<","<<0<<");";

        char * errmsg;
        int ec = sqlite3_exec(db,ss.str().c_str(),NULL,NULL,&errmsg);
        if (ec != SQLITE_OK) {
            std::cout<<"failed to add player to DB "<<errmsg<<"\n";
            sqlite3_free(errmsg);
        }
        return value.first->second;
    }

    //calculate and check a message and update game data
    void Bot::CountingGame::count(dpp::cluster &bot, const dpp::message_create_t &message) {
        if (message.msg->channel_id == channelID) {
            if(!shouldCount(message.msg->content)) {
                return;
            }
            const dpp::snowflake author = message.msg->author->id;
            if (currentCount != resetCount && lastPlayer != nullptr && author == lastPlayer->userId) {
                reply(CountingGame::Type::SAME_USER, bot, message,0);
                auto keySet = players.find(author);
                if (keySet != players.end()) {
                    addCountToPlayer(keySet->second,false);
                    saveGame();
                }
                return;
            }
            std::string_view view(message.msg->content);
            auto RPNList = StringCalculator::convertStringToRPNList(view);
            double value = StringCalculator::calculateFromRPNList(RPNList);
            value = std::floor(value);
            bool isCorrect = value == ++currentCount;
            auto keySet = players.find(author);
            if (keySet != players.end()) {
                reply(addCountToPlayer(keySet->second, isCorrect), bot, message,value);
                saveGame();
                return;
            } else {
                auto player = addPlayer(author);
                reply(addCountToPlayer(player, isCorrect), bot, message,value);
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
                        "> Cant count.Gave Value "+std::to_string(cast)+". Count reset to 1"
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
        const char * index = input.data();
        const char * end = input.data()+input.size();
        while (index < end) {
            if (*index == ' ') {
                index++;
                continue;
            }
            if (*index == '(') return true;
            char32_t unicode;
            index = StringCalculator::getUnicode(index,unicode);
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
                      return p1->second.getTotalCount() >= p2->second.getTotalCount();
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
}










/*




 calculateString(string):
    for (char : string):
        if char == number:




25*5(69+25)+25

queue
 25
 *5
 * (69+25)
 +25

25+(5*9+3)*5


 */