//
// Created by bot on 11/22/22.
//

#pragma once

#include "sqlite3.h"
#include <string>

namespace Sqlitepp {
    class Database {
    public:
        Database(const char *database);

        Database(const std::string database);

        void exec(const char *statment, int (*callback)(void*,int,char**,char**), void *data);

        void exec(const std::string statment, int (*callback)(void*,int,char**,char**), void *data);

        void exec(const char* statment);

        void exec(const std::string statment);

        bool tableExists(const std::string name);

        ~Database() {
            sqlite3_close(db);
        }


    private:
        sqlite3 * db;
    };

} // Sqlitepp
