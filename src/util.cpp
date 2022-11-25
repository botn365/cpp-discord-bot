//
// Created by bot on 11/24/22.
//

#include "../include/util.hpp"

std::string Bot::Util::iFileToString(std::string file) {
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
