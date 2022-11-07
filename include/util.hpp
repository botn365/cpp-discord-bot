//
// Created by bot on 10/17/22.
//
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Bot::Util {
    std::string iFileToString(std::string file) {
        std::ifstream inputStream(file);
        if (!inputStream.is_open()) {
            std::ofstream genFile(file);
            std::cout<<"file "<<file<<" not found\n generating file "<<file<<"\n";
            if (genFile.is_open()) {
                genFile.close();
            }
            return "{}";
        }
        std::stringstream ss;
        ss << inputStream.rdbuf();
        return ss.str();
    }
}