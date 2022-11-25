//
// Created by bot on 11/22/22.
//

#include "../include/database.hpp"

#include <stdexcept>

namespace Sqlitepp {
    Database::Database(const char *database) {
        int error = sqlite3_open(database,&db);
        if (error != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }

    void Database::exec(const char *statment, int (*callback)(void *, int, char **, char **), void *data) {
//        char *errmsg;
//        int error = sqlite3_exec(db,statment,callback,data,&errmsg);
//        if (error != SQLITE_OK) {
//            std::string er{errmsg};
//            sqlite3_free(errmsg);
//            throw std::runtime_error(er);
//        }
    }

    void Database::exec(const std::string statment, int (*callback)(void *, int, char **, char **), void *data) {
        exec(statment.c_str(),callback,data);
    }

    void Database::exec(const char *statment) {
        exec(statment, nullptr, nullptr);
    }

    void Database::exec(const std::string statment) {
        exec(statment.c_str());
    }

    Database::Database(const std::string database) : Database(database.c_str()) {}

    bool Database::tableExists(const std::string name) {
        return false;
    }


} // Sqlitepp