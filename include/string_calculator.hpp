//
// Created by vanda on 12/10/2021.
//

#pragma once

#include <functional>
#include <stack>
#include <list>
#include <utility>
#include <memory>
#include <unordered_map>
#include <string>

namespace Bot {
    class CountObj {
    public:
        virtual bool isOperator() = 0;
    };

    class Number : public CountObj {
    public:
        explicit Number(double value) : value{value} {}

        bool isOperator() override { return false; }

        double value;
    };

    class Operator : public CountObj {
    public:
        Operator(int priority, std::string unicode, std::function<bool(std::stack<double> &)> run,
                 bool canHaveNumber = true, bool isReversed = false) :
                priority{priority}, name{std::move(unicode)}, run{std::move(run)} {
            if (canHaveNumber) {
                extraData |= 1;
                if (isReversed) {
                    extraData |= 2;
                }
            }
        }

        bool isOperator() override { return true; }

        [[nodiscard]] bool canHaveNumber() const {
            return extraData & 1;
        }

        [[nodiscard]] bool isReversed() const {
            return extraData & 2;
        }

        std::function<bool(std::stack<double> &)> run;
        std::string name;
        char extraData = 0;
        int priority;
    };

    struct Function : public Operator {
        Function(int priority, std::string name, const std::function<bool(std::stack<double> &)> &run,
                 bool canHaveNumber, bool isReversed) : Operator(priority, std::move(name), run, canHaveNumber,
                                                                 isReversed) {};

    };


    class StringCalculator {
    public:
        static void init(std::string numeTranslationFile);

        static std::list<std::unique_ptr<CountObj>> convertStringToRPNList(std::string_view &input);

        static double calculateFromRPNList(std::list<std::unique_ptr<CountObj>> &list);

        static void addOperator(std::string unicode, int priority, std::function<bool(std::stack<double> &)> run,
                                bool canHaveNumber = true, bool isReversed = false);

        static void addFunction(std::string, int priority, std::function<bool(std::stack<double> &)> run,
                                bool canHaveNumber = true, bool isReversed = true);

        static void addUnicodeNumber(char32_t unicode, int value);

        static bool getNumber(const char **input, double &number, const char *end);

        static int getNumberFromUnicode(const char32_t &unicode);

        static const char *getUnicode(const char *input, char32_t &unicode);

        static Operator *getOperator(char32_t unicode);

        static std::string unicodeToString(char32_t unicode);

        static bool isUnicodeUsed(char32_t unicode);

        static std::string_view getFunctionString(const char **currentChar, const char *end);

        static double getConst(std::string_view &view);

        static bool hasFunction(std::string &str);

        static bool hasConst(std::string &str);

        static double floor(double in);

        static std::unordered_map<char32_t, Bot::Operator> getOperatorMap();

        static std::unordered_map<char32_t, int> getNumberMap();

        static std::unordered_map<std::string, Bot::Function> getFunctionMap();

        static std::unordered_map<std::string, double> getConstMap();

    private:
        using list = std::list<std::unique_ptr<CountObj>>;

        static bool shouldCommaIndexUp(list::iterator &index, uint64_t bracketPriorety, list &list);

        static int getParanthese(char32_t &unicode);

        static void
        insertOperatorInRPNList(list &list, list::iterator &index, Operator *operand, int paranthesePriorety);

        static char32_t convertToUnicode(const char *input, int len);
    };
}



