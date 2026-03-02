#include "Database.hpp"
#include <iostream>

Database::Database() {}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: [Database] Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

void Database::disconnect() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

sqlite3* Database::getHandle() {
    return db;
}

bool Database::execute(const std::string& sql) {
    if (!db) return false;
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Error: [Database] SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool Database::commit() {
    return execute("COMMIT;");
}

bool Database::rollback() {
    return execute("ROLLBACK;");
}