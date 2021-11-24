//
// Created by vanda on 04/10/2021.
//

#include "../include/player.hpp"

Bot::Player::Player(dpp::snowflake userId) : userId{userId} {}

Bot::Player::Player(dpp::snowflake userId, uint64_t totalCorrect, uint64_t totalFailed, uint64_t highestCount, uint64_t saves) :
        Player(userId)
{
    this->totalCorrect = totalCorrect;
    this->totalFailed = totalFailed;
    this->highestCount = highestCount;
    totalCount = totalCorrect + totalFailed;
    this->saves = saves;
}

double Bot::Player::getSuccessRate() const {
    return (static_cast<double>(totalCorrect) / static_cast<double>(totalCount));
}

uint64_t Bot::Player::getTotalCount() const {
    return totalCount;
}

void Bot::Player::incrementFailedCount() {
    totalCount++;
    totalFailed++;
}

void Bot::Player::incrementCorrectCount() {
    totalCount++;
    totalCorrect++;
}

void Bot::Player::checkAndSetHighestCount(double value) {
    if (value > highestCount) {
        highestCount = value;
    }
}
