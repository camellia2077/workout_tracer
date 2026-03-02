#include "SqliteManager.h"
#include "sqlite3.h"
#include <iostream>
#include <sstream>

SqliteManager::SqliteManager() {}

SqliteManager::~SqliteManager() {
    disconnect();
}

bool SqliteManager::connect(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: [SqliteManager] Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return initializeSchema();
}

void SqliteManager::disconnect() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool SqliteManager::initializeSchema() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS DailyLogs ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  log_date TEXT NOT NULL UNIQUE"
        ");"
        "CREATE TABLE IF NOT EXISTS Projects ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  daily_log_id INTEGER NOT NULL,"
        "  project_name TEXT NOT NULL,"
        "  weight REAL NOT NULL,"
        "  reps_list TEXT NOT NULL,"
        "  volume REAL NOT NULL,"
        "  FOREIGN KEY(daily_log_id) REFERENCES DailyLogs(id)"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Error: [SqliteManager] Failed to create schema: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool SqliteManager::saveData(const std::vector<DailyData>& processedData) {
    if (!db) return false;

    char* errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errMsg) != SQLITE_OK) {
         std::cerr << "Error: [SqliteManager] Could not begin transaction: " << errMsg << std::endl;
         sqlite3_free(errMsg);
         return false;
    }

    for (const auto& daily : processedData) {
        std::string sql_insert_date = "INSERT OR IGNORE INTO DailyLogs (log_date) VALUES ('" + daily.date + "');";
        if (sqlite3_exec(db, sql_insert_date.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "Error: [SqliteManager] SQL error on date insert: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
            return false;
        }

        std::string sql_select_id = "SELECT id FROM DailyLogs WHERE log_date = '" + daily.date + "';";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql_select_id.c_str(), -1, &stmt, 0);
        
        long long daily_log_id = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            daily_log_id = sqlite3_column_int64(stmt, 0);
        }
        sqlite3_finalize(stmt);

        if (daily_log_id == -1) {
             sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
             return false;
        }

        for (const auto& project : daily.projects) {
            std::stringstream reps_ss;
            for (size_t i = 0; i < project.reps.size(); ++i) {
                reps_ss << project.reps[i] << (i < project.reps.size() - 1 ? "," : "");
            }

            std::stringstream sql_insert_project;
            sql_insert_project << "INSERT INTO Projects (daily_log_id, project_name, weight, reps_list, volume) VALUES ("
                               << daily_log_id << ", '"
                               << sqlite3_mprintf("%q", project.projectName.c_str()) << "', " // Use %q for safe string insertion
                               << project.weight << ", '"
                               << reps_ss.str() << "', "
                               << project.volume << ");";
            
            std::string sql = sql_insert_project.str();
            if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
                std::cerr << "Error: [SqliteManager] Failed to insert project: " << errMsg << std::endl;
                sqlite3_free(errMsg);
                sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
                return false;
            }
        }
    }

    if (sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Error: [SqliteManager] Failed to commit transaction: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}