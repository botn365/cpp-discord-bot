//
// Created by vanda on 13/10/2021.
//

#pragma once

#include "string_calculator.hpp"
#include <rapidjson/document.h>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cmath>


namespace Bot {

    using Str = Bot::StringCalculator;

    class LoadOperators {
    public:
        static void loadNumbers(const std::string &translationFile) {
            auto input = fileToString(translationFile);
            rapidjson::Document doc;
            doc.Parse(input.c_str());
            if (doc.HasParseError()) {
                std::stringstream ss;
                ss << "failed to parse json ParseErrorCode = " << doc.GetParseError();
                throw std::runtime_error(ss.str());
            }
            try {
                auto &codes = doc["codes"];
                for (auto &code: codes.GetArray()) {
                    Str::addUnicodeNumber(code["unicode"].GetInt(),code["value"].GetInt());
                }
            } catch (std::exception &e){
                throw std::runtime_error("failed to get values from json");
            }
        }

        static void loadOperators() {
            Str::addOperator('+', 1, [](std::stack<double> &stack) {
                if (stack.size() < 2) return false;
                double d1 = stack.top();
                stack.pop();
                double d2 = stack.top();
                stack.pop();
                stack.push(d1 + d2);
                return true;
            });
            Str::addOperator('-', 1, [](std::stack<double> &stack) {
                if (stack.size() < 2) return false;
                double d1 = stack.top();
                stack.pop();
                double d2 = stack.top();
                stack.pop();
                stack.push(d2-d1);
                return true;
            });
            Str::addOperator('*', 2, [](std::stack<double> &stack) {
                if (stack.size() < 2) return false;
                double d1 = stack.top();
                stack.pop();
                double d2 = stack.top();
                stack.pop();
                stack.push(d1 * d2);
                return true;
            });
            Str::addOperator('/', 2, [](std::stack<double> &stack) {
                if (stack.size() < 2) return false;
                double d1 = stack.top();
                stack.pop();
                double d2 = stack.top();
                stack.pop();
                stack.push(d2/d1);
                return true;
            });
            Str::addOperator('^', 3, [](std::stack<double> &stack) {
                if (stack.size() < 2) return false;
                double d1 = stack.top();
                stack.pop();
                double d2 = stack.top();
                stack.pop();
                stack.push(pow(d2,d1));
                return true;
            });

            Str::addOperator(0xE2889A, 3, [](std::stack<double> &stack) {
                if (stack.size() < 1) return false;
                double d1 = stack.top();
                stack.pop();
                stack.push(sqrt(d1));
                return true;
            });

            Str::addOperator('~',4,[](std::stack<double> &stack){
                if (stack.size()<1) return false;
                double d1 = stack.top();
                stack.pop();
                stack.push(-d1);
                return true;
            });

            Str::addOperator('|' ,0,[](std::stack<double> &stack){
                if (stack.size()< 2) return false;
                int64_t d1 = stack.top();
                stack.pop();
                int64_t d2 = stack.top();
                stack.pop();
                stack.push(d2 | d1);
                return true;
            });

            Str::addOperator('&' ,0,[](std::stack<double> &stack){
                if (stack.size()< 2) return false;
                int64_t d1 = stack.top();
                stack.pop();
                int64_t d2 = stack.top();
                stack.pop();
                stack.push(d2 & d1);
                return true;
            });

            Str::addOperator('\\' ,0,[](std::stack<double> &stack){
                if (stack.size()< 2) return false;
                int64_t d1 = stack.top();
                stack.pop();
                int64_t d2 = stack.top();
                stack.pop();
                stack.push(d2 ^ d1);
                return true;
            });

            Str::addOperator('<' ,0,[](std::stack<double> &stack){
                if (stack.size()< 2) return false;
                int64_t d1 = stack.top();
                stack.pop();
                int64_t d2 = stack.top();
                stack.pop();
                stack.push(d2 << d1);
                return true;
            });

            Str::addOperator('>' ,0,[](std::stack<double> &stack){
                if (stack.size()< 2) return false;
                int64_t d1 = stack.top();
                stack.pop();
                int64_t d2 = stack.top();
                stack.pop();
                stack.push(d2 >> d1);
                return true;
            });

            Str::addOperator('!', 3, [](std::stack<double> &stack) {
                if (stack.size() < 1) return false;
                int64_t d1 = stack.top();
                stack.pop();
                if (d1 < 0) return false;
                double fac = 1.0;
                for (int64_t i = 2;i<d1;i++) {
                    fac *= i;
                }
                stack.push(fac);
                return true;
            });
        }

        static std::string fileToString(std::string file) {
            std::ifstream inputStream(file);
            if (!inputStream.is_open()) {
                throw std::runtime_error(file+" not found");
            }
            std::stringstream ss;
            ss << inputStream.rdbuf();
            return ss.str();
        }
    };
}