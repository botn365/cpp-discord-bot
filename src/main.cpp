
#include "../include/app.hpp"

int main() {
    Bot::App app;
    try {
        app.run();
    } catch (std::exception &e) {
        std::cout<<"fatal exception what() "<<e.what()<<"\n";
    }
    return 0;
}