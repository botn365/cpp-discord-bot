//
// Created by vanda on 12/10/2021.
//

#include <cmath>
#include "../include/string_calculator.hpp"

static std::unordered_map<char32_t, Bot::Operator> unicodeToOperator;
static std::unordered_map<char32_t, int> unicodeToNumber;


std::list<std::unique_ptr<Bot::CountObj>> Bot::StringCalculator::convertStringToRPNList(std::string_view &input) {
    std::list<std::unique_ptr<CountObj>> list;
    list.push_back(std::make_unique<Number>(0));
    const char *end = input.data() + input.size();
    auto index = list.begin();
    int bracketPriorety = 0;
    bool indexUp = false;
    bool numberWasLast = false;
    auto *multOp = getOperator('*');
    for (const char *i = input.data(); i < end;) {
        double number;
        if (getNumber(&i, number, end)) {
            if (numberWasLast) {
                insertOperatorInRPNList(list,index,multOp, bracketPriorety);
            }
            index = list.insert(++index, std::make_unique<Number>(number));
            indexUp = true;
            numberWasLast = true;
        } else {
            char32_t unicode;
            i = getUnicode(i, unicode);
            if (unicode == 0) continue;
            if (!numberWasLast && unicode == '-') unicode = '~';
            auto operatorLambda = getOperator(unicode);
            if (operatorLambda != nullptr) {
                insertOperatorInRPNList(list, index, operatorLambda, bracketPriorety);
                indexUp = true;
                numberWasLast = false;
                if (unicode == '!') index++;
            } else {
                int bracket = getParanthese(unicode);
                if (bracket > 0 && indexUp) {
                    indexUp = false;
                    numberWasLast = false;
                    index--;
                }
                if (bracket < 0){
                    indexUp = false;
                    numberWasLast = false;
                }
                bracketPriorety += bracket;
                if (unicode != ' ') break;
            }
        }
    }
    list.pop_front();
    return list;
}

double Bot::StringCalculator::calculateFromRPNList(std::list<std::unique_ptr<CountObj>> &inputList) {
    std::stack<double> stack;
    for (std::unique_ptr<CountObj> &value: inputList) {
        if (value->isOperator()) {
            Operator *op = (Operator *) (value.get());
            if (!op->run(stack)) {
                return NAN;
            }
        } else {
            Number *num = (Number *) (value.get());
            stack.push(num->value);
        }
    }
    if (stack.size() != 1 ) return NAN;
    return stack.top();
}

//add unicode operator pair to hashmap
void Bot::StringCalculator::addOperator(char32_t unicode, int priorety, std::function<bool(std::stack<double> &)> run) {
    unicodeToOperator.insert(std::pair<char32_t, Operator>(unicode, Operator(priorety, run)));
}

//add unicode digit pair to hashmap
void Bot::StringCalculator::addUnicodeNumber(char32_t unicode, int value) {
    unicodeToNumber.insert(std::pair<char32_t, int>(unicode, value));
}

int Bot::StringCalculator::getParanthese(char32_t &unicode) {
    if (unicode == '(') {
        return 10;
    } else if (unicode == ')') {
        return -10;
    }
    return 0;
}

//inserts an operator in to the list
void
Bot::StringCalculator::insertOperatorInRPNList(std::list<std::unique_ptr<CountObj>> &list,
                                               std::list<std::unique_ptr<CountObj>>::iterator &index,
                                               Bot::Operator *operand, int paranthesePriorety) {
    while (index != list.end()) {
        if ((*index)->isOperator()) {
            auto *op = (Operator *) (*index).get();
            int priorety = op->priorety;
            if (priorety < operand->priorety + paranthesePriorety) {
                index = list.insert(index,
                                    std::make_unique<Operator>(operand->priorety + paranthesePriorety, operand->run));
                index--;
                return;
            }
        }
        index++;
    }
    index = list.insert(index, std::make_unique<Operator>(operand->priorety + paranthesePriorety, operand->run));
    index--;
}

Bot::Operator *Bot::StringCalculator::getOperator(char32_t unicode) {
    auto result = unicodeToOperator.find(unicode);
    if (result == unicodeToOperator.end()) return nullptr;
    return &result->second;
}

int Bot::StringCalculator::getNumberFromUnicode(const char32_t &unicode) {
    auto result = unicodeToNumber.find(unicode);
    if (result == unicodeToNumber.end()) return -1;
    return result->second;
}


bool Bot::StringCalculator::getNumber(const char **input, double &number, const char *end) {
    number = 0;
    bool comma = false;
    bool hasNumber = false;
    unsigned int multiplyer = 10;
    while (*input < end) {
        char32_t unicode;
        const char *value = getUnicode(*input, unicode);
        int result = getNumberFromUnicode(unicode);
        if (result == -1) {
            if (unicode == ',' || unicode == '.') {
                *input = value;
                comma = true;
            } else {
                break;
            }
        } else {
            *input = value;
            if (comma) {
                number += ((double) result / multiplyer);
                multiplyer *= 10;
            } else {
                number *= multiplyer;
                number += result;
                hasNumber = true;
            }
        }
    }
    return hasNumber;
}

const char *Bot::StringCalculator::getUnicode(const char *input, char32_t &unicode) {
    if ((*input & 0x80) == 0) {
        unicode = *input;
        return ++input;
    } else if ((*input & 0xE0) == 192) {
        unicode = converToUnicode(input, 2);
        return input += 2;
    } else if ((*input & 0xF0) == 224) {
        unicode = converToUnicode(input, 3);
        return input += 3;
    } else if ((*input & 0xF8) == 240) {
        unicode = converToUnicode(input, 4);
        return input += 4;
    }
    unicode = 0;
    return ++input;
}

//converts a char* to unicode while checking if it is formed correctly
char32_t Bot::StringCalculator::converToUnicode(const char *input, int len) {
    unsigned char value = *input;
    char32_t out = value;
    for (int i = 1; i < len; i++) {
        if ((*(input + i) & 0xc) == 8) {
            out <<= 8;
            value = *(input + i);
            out += value;
        } else {
            return 0;
        }
    }
    return out;
}
