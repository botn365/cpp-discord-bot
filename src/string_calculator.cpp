//
// Created by vanda on 12/10/2021.
//

#include <cmath>
#include "../include/string_calculator.hpp"
#include "../include/load_operators.hpp"

static std::unordered_map<char32_t, Bot::Operator> unicodeToOperator;
static std::unordered_map<char32_t, int> unicodeToNumber;
static std::unordered_map<char32_t, bool> usedUnicodeMap;
static std::unordered_map<std::string, double> constMap;
static std::unordered_map<std::string, Bot::Function> stringToFunction;


namespace Bot {

    using usedPair = std::pair<char32_t, bool>;
    using constPair = std::pair<std::string, double>;

    void StringCalculator::init(std::string numeTranslationFile) {
        Bot::LoadOperators::loadNumbers(numeTranslationFile);
        Bot::LoadOperators::loadOperators();
        usedUnicodeMap.insert(usedPair('(', true));
        usedUnicodeMap.insert(usedPair(')', true));
        usedUnicodeMap.insert(usedPair('{', true));
        usedUnicodeMap.insert(usedPair('}', true));
        usedUnicodeMap.insert(usedPair(' ', true));
        usedUnicodeMap.insert(usedPair('.', true));
        usedUnicodeMap.insert(usedPair(',', true));
        constMap.insert(constPair("pi", M_PI));
        constMap.insert(constPair("e", M_E));
        constMap.insert(constPair("g", 9.8));
        constMap.insert(constPair("π", 3));
        constMap.insert(constPair("een", 1));
        constMap.insert(constPair("twee", 2));
        constMap.insert(constPair("drie", 3));
        constMap.insert(constPair("vier", 4));
        constMap.insert(constPair("vijf", 5));
        constMap.insert(constPair("zes", 6));
        constMap.insert(constPair("zeven", 7));
        constMap.insert(constPair("acht", 8));
        constMap.insert(constPair("negen", 9));
    }

    std::list<std::unique_ptr<Bot::CountObj>> Bot::StringCalculator::convertStringToRPNList(std::string_view &input) {
        std::list<std::unique_ptr<CountObj>> list;
        list.push_back(std::make_unique<Number>(0));
        const char *end = input.data() + input.size();
        auto index = list.begin();
        int bracketPriorety = 0;
        bool indexUp = false;
        bool numberWasLast = false;
        auto *multOp = getOperator('*');
        std::stack<std::pair<uint64_t, Function *>> functionStack;
        for (const char *i = input.data(); i < end;) {
            if (list.size() > 200) return {};
            double number;
            if (getNumber(&i, number, end)) {
                if (numberWasLast) {
                    insertOperatorInRPNList(list, index, multOp, bracketPriorety);
                }
                index = list.insert(++index, std::make_unique<Number>(number));
                indexUp = false;
                numberWasLast = true;
            } else {
                char32_t unicode;
                const char *newIPos = getUnicode(i, unicode);
                if (unicode == 0) {
                    i = newIPos;
                    continue;
                }
                if (!numberWasLast && unicode == '-') unicode = '~';
                auto operatorLambda = getOperator(unicode);
                if (operatorLambda != nullptr) {
                    if (numberWasLast && !operatorLambda->canHaveNumber()) {
                        insertOperatorInRPNList(list, index, multOp, bracketPriorety);
                    }
                    insertOperatorInRPNList(list, index, operatorLambda, bracketPriorety);
                    if (operatorLambda->isReversed()) {
                        index++;
                        indexUp = false;
                    } else {
                        numberWasLast = false;
                        indexUp = false;
                    }
                    i = newIPos;
                } else {
                    int bracket = getParanthese(unicode);
                    if (bracket > 0) {
                        if (numberWasLast) {
                            insertOperatorInRPNList(list, index, multOp, bracketPriorety);
                            numberWasLast = false;
                        }
                        if (indexUp) {
                            index--;
                            indexUp = false;
                        }
                        bracketPriorety += bracket;
                        i = newIPos;
                        continue;
                    }
                    if (bracket < 0) {
                        indexUp = false;
                        bracketPriorety += bracket;
                        i = newIPos;
                        if (!functionStack.empty() && functionStack.top().first == bracketPriorety) {
                            operatorLambda = functionStack.top().second;
                            insertOperatorInRPNList(list, index, operatorLambda, bracketPriorety);
                            functionStack.pop();
                            if (operatorLambda->isReversed()) {
                                index++;
                                indexUp = false;
                            } else {
                                numberWasLast = false;
                                indexUp = false;
                            }
                        }
                        continue;
                    }
                    if (unicode == ',') {
                        do {
                            index++;
                        } while (shouldCommaIndexUp(index, bracketPriorety, list));
                        index--;
                        numberWasLast = false;
                        i = newIPos;
                        continue;
                    }
                    if (unicode != ' ') {
                        auto view = getFunctionString(&i, end);
                        if (*(i) == '(') {
                            auto functionPair = stringToFunction.find(std::string{view});
                            if (functionPair != stringToFunction.end()) {
                                functionStack.push(
                                        std::pair<uint64_t, Function *>(bracketPriorety, &functionPair->second));
                                continue;
                            }
                            break;
                        } else {
                            auto value = getConst(view);
                            if (value == value) {
                                if (numberWasLast) {
                                    insertOperatorInRPNList(list, index, multOp, bracketPriorety);
                                }
                                index = list.insert(++index, std::make_unique<Number>(value));
                                indexUp = true;
                                numberWasLast = true;
                                continue;
                            }
                            break;
                        }
                    }
                    i = newIPos;
                }
            }
        }
        list.pop_front();
        return list;
    }

    using list = std::list<std::unique_ptr<CountObj>>;

    bool StringCalculator::shouldCommaIndexUp(list::iterator &index, uint64_t bracketPriorety, list &list) {
        if (index == list.end()) return false;
        CountObj *ptr = (*index).get();
        if (ptr->isOperator()) {
            Operator *opPtr = (Operator *) ptr;
            if (opPtr->priority >= bracketPriorety) return true;
        }
        return false;
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
        if (stack.size() != 1) return NAN;
        return stack.top();
    }

//add name operator pair to hashmap
    void
    Bot::StringCalculator::addOperator(std::string unicode, int priority, std::function<bool(std::stack<double> &)> run,
                                       bool canHaveNumber, bool isReversed) {

        char32_t unicodeValue;
        getUnicode(unicode.c_str(), unicodeValue);
        unicodeToOperator.insert(
                std::pair<char32_t, Operator>(unicodeValue,
                                              Operator(priority, unicode, run, canHaveNumber, isReversed)));
        auto used = usedUnicodeMap.find(unicodeValue);
        if (used == usedUnicodeMap.end()) {
            usedUnicodeMap.insert(usedPair(unicodeValue, false));
        }
    }

    void StringCalculator::addFunction(std::string key, int priority, std::function<bool(std::stack<double> &)> run,
                                       bool canHaveNumber, bool isReversed) {
        stringToFunction.insert(
                std::pair<std::string, Function>(key, Function(priority, key, run, canHaveNumber, isReversed)));
    }

//add name digit pair to hashmap
    void Bot::StringCalculator::addUnicodeNumber(char32_t unicode, int value) {
        unicodeToNumber.insert(std::pair<char32_t, int>(unicode, value));
        usedUnicodeMap.insert(std::pair<char32_t, int>(unicode, true));
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
        index++;
        while (index != list.end()) {
            if ((*index)->isOperator()) {
                auto *op = (Operator *) (*index).get();
                int priorety = op->priority;
                if (priorety < operand->priority + paranthesePriorety) {
                    index = list.insert(index,
                                        std::make_unique<Operator>(operand->priority + paranthesePriorety,
                                                                   operand->name, operand->run));
                    index--;
                    return;
                }
            }
            index++;
        }
        index = list.insert(index, std::make_unique<Operator>(operand->priority + paranthesePriorety, operand->name,
                                                              operand->run));
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
        const char *old = nullptr;
        while (*input < end) {
            char32_t unicode;
            const char *value = getUnicode(*input, unicode);
            if (value == old) return false;
            old = value;
            int result = getNumberFromUnicode(unicode);
            if (result == -1) {
                if (unicode == '_') {
                    *input = value;
                    continue;
                }
                if (unicode == '.') {
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
            unicode = convertToUnicode(input, 2);
            return input += 2;
        } else if ((*input & 0xF0) == 224) {
            unicode = convertToUnicode(input, 3);
            return input += 3;
        } else if ((*input & 0xF8) == 240) {
            unicode = convertToUnicode(input, 4);
            return input += 4;
        }
        unicode = 0;
        return ++input;
    }

//converts a char* to name while checking if it is formed correctly
    char32_t Bot::StringCalculator::convertToUnicode(const char *input, int len) {
        unsigned char value = *input;
        char32_t out = value;
        for (int i = 1; i < len; i++) {
            if (i == 0 || (*(input + i) & 0xc0) == 0x80) {
                out <<= 8;
                value = *(input + i);
                out += value;
            } else {
                return 0;
            }
        }
        return out;
    }

    std::string StringCalculator::unicodeToString(char32_t unicode) {
        std::string out;
        for (int i = 3; i >= 0; i--) {
            unsigned char ch = (unicode >> (i * 8)) % 256;
            if (ch != 0) {
                out += ch;
            }
        }
        return out;
    }

    bool StringCalculator::isUnicodeUsed(char32_t unicode) {
        return usedUnicodeMap.find(unicode) != usedUnicodeMap.end();
    }

    std::string_view Bot::StringCalculator::getFunctionString(const char **currentChar, const char *end) {
        const char *newPos = *currentChar;
        while (newPos != end) {
            char32_t unicode;
            const char *tempPos = getUnicode(newPos, unicode);
            if (usedUnicodeMap.find(unicode) != usedUnicodeMap.end()) {
                std::string_view view(*currentChar, newPos - *currentChar);
                *currentChar = newPos;
                return view;
            } else {
                newPos = tempPos;
            }
        }
        std::string_view view(*currentChar, end - *currentChar);
        *currentChar = newPos;
        return view;
    }

    double Bot::StringCalculator::getConst(std::string_view &view) {
        auto value = constMap.find(std::string(view));
        if (value != constMap.end()) {
            return value->second;
        }
        return NAN;
    }

    bool Bot::StringCalculator::hasFunction(std::string &str) {
        return stringToFunction.find(str) != stringToFunction.end();
    }

    bool Bot::StringCalculator::hasConst(std::string &str) {
        return constMap.find(str) != constMap.end();
    }

    double StringCalculator::floor(double in) {
        return std::floor(in + 0.000001);
    }

    std::unordered_map<char32_t, Bot::Operator> StringCalculator::getOperatorMap() {
        return unicodeToOperator;
    }

    std::unordered_map<char32_t, int> StringCalculator::getNumberMap() {
        return unicodeToNumber;
    }

    std::unordered_map<std::string, Bot::Function> StringCalculator::getFunctionMap() {
        return stringToFunction;
    }

    std::unordered_map<std::string, double> StringCalculator::getConstMap() {
        return constMap;
    }
}

