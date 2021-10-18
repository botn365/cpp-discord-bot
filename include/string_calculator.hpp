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


namespace Bot {
    class CountObj {
    public:
        virtual bool isOperator() = 0;
    };

    class Number : public CountObj {
    public:
        Number(double value) : value{value} {}

        bool isOperator() override { return false; }

        double value;
    };

    class Operator : public CountObj {
    public:
        Operator(int priorety, std::function<bool(std::stack<double>&)> run) :
                priorety{priorety}, run{std::move(run)} {}

        bool isOperator() override { return true; }

        std::function<bool(std::stack<double>&)> run;
        int priorety;
    };

    class StringCalculator {
    public:
        static std::list<std::unique_ptr<CountObj>> convertStringToRPNList(std::string_view &input);

        static double calculateFromRPNList(std::list<std::unique_ptr<CountObj>> &list);

        static void addOperator(char32_t unicode, int priorety, std::function<bool(std::stack<double>&)> run);

        static void addUnicodeNumber(char32_t unicode, int value);

        static bool getNumber(const char **input,double &number,const char * end);

        static int getNumberFromUnicode(const char32_t &unicode);

        static const char *getUnicode(const char *input,char32_t &unicode);

    private:
        static int getParanthese(char32_t &unicode);

        static void insertOperatorInRPNList(std::list<std::unique_ptr<CountObj>> &list,
                                            std::list<std::unique_ptr<CountObj>>::iterator &index,
                                            Operator *operand, int paranthesePriorety);

        static Operator *getOperator(char32_t unicode);

        static char32_t converToUnicode(const char *input,int len);
    };
}



