// src/db/DbManager.cpp

#include "DbManager.hpp"
#include <iostream>

DbManager::DbManager(const std::string& dbPath) : dbPath_(dbPath) {}

DbManager::~DbManager() {
    close();
}

bool DbManager::open() {
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    std::cout << "Database opened successfully at " << dbPath_ << std::endl;
    return createTables();
}

void DbManager::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
        std::cout << "Database connection closed." << std::endl;
    }
}

sqlite3* DbManager::getConnection() const {
    return db_;
}

// [MODIFIED] 更新了 training_logs 表的结构
bool DbManager::createTables() {
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS training_logs ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  cycle_id TEXT NOT NULL,"            // <<< 新增: 训练周期的唯一ID
        "  total_days INTEGER NOT NULL,"       // <<< 新增: 该周期的总训练天数
        "  date TEXT NOT NULL,"
        "  exercise_name TEXT NOT NULL,"
        "  exercise_type TEXT NOT NULL,"
        "  total_volume REAL NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS training_sets ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  log_id INTEGER NOT NULL,"
        "  set_number INTEGER NOT NULL,"
        "  weight REAL NOT NULL,"
        "  reps INTEGER NOT NULL,"
        "  volume REAL NOT NULL,"
        "  FOREIGN KEY (log_id) REFERENCES training_logs (id)"
        ");";

    char* zErrMsg = nullptr;
    if (sqlite3_exec(db_, sql, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error creating tables: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    
    std::cout << "Tables verified/created successfully." << std::endl;
    return true;
}