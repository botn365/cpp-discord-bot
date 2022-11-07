//
// Created by bot on 10/12/22.
//

#include "../include/server.hpp"
#include "app.cpp"

namespace Bot {

    Server::Server(int id, App &app) : app(app), serverID(id) {
        folder = "servers/"+std::to_string(id);
        settings = std::make_unique<ServerSettings>(folder+"/settings");
    }

    Server::Server(std::string folder, App &app) : app(app) {
        this->folder = folder;
        this->settings = std::make_unique<ServerSettings>(folder+"/settings");
        serverID = this->settings->getServerId();
    }

    void Server::init() {
        countingGame = std::make_unique<CountingGame>(*this);
    }

    void Server::registerCallbacks() {

    }

    dpp::cluster &Server::getBot() const {
        return *app.bot.get();
    }
} // Bot