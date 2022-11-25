//
// Created by vanda on 04/10/2021.
//

#pragma once

#include <dpp/dpp.h>

namespace Bot {
    class Player {
    public:
        Player(dpp::snowflake userId);

        Player(dpp::snowflake userId,uint64_t totalCorrect, uint64_t totalFailed, uint64_t highestCount, uint64_t saves);

        void incrementFailedCount();

        void incrementCorrectCount();

        double getSuccessRate() const;

        uint64_t getTotalCount() const;

        uint64_t getCorectCount() const {return totalCorrect;}

        uint64_t getFailedCount() const {return totalFailed;}

        uint64_t getHighestCount() const {return highestCount;}

        uint64_t getSaves() const {return saves;}

        void setSaves(uint64_t saves) {this->saves=saves;}

        const dpp::snowflake userId;

        void checkAndSetHighestCount(double value);
    private:
        uint64_t totalCorrect = 0;
        uint64_t totalFailed = 0;
        uint64_t totalCount = 0;
        uint64_t highestCount = 0;
        uint64_t saves = 0;
    };
}



