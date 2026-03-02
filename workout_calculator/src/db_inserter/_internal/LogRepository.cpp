#include "LogRepository.hpp"
#include "sqlite3.h" // 需要用于预处理语句相关的API
#include <iostream>
#include <sstream>

LogRepository::LogRepository(Database& db) : database(db) {}

bool LogRepository::initializeSchema() {
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
    
    if (!database.execute(sql)) {
        std::cerr << "Error: [LogRepository] Failed to create schema." << std::endl;
        return false;
    }
    return true;
}

bool LogRepository::save(const std::vector<DailyData>& processedData) {
    sqlite3* dbHandle = database.getHandle();
    if (!dbHandle) return false;

    if (!database.beginTransaction()) {
        std::cerr << "Error: [LogRepository] Could not begin transaction." << std::endl;
        return false;
    }

    // 为插入和查询准备预处理语句
    const char* sql_insert_date = "INSERT OR IGNORE INTO DailyLogs (log_date) VALUES (?);";
    const char* sql_select_id = "SELECT id FROM DailyLogs WHERE log_date = ?;";
    const char* sql_insert_project = "INSERT INTO Projects (daily_log_id, project_name, weight, reps_list, volume) VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt_insert_date, *stmt_select_id, *stmt_insert_project;

    if (sqlite3_prepare_v2(dbHandle, sql_insert_date, -1, &stmt_insert_date, 0) != SQLITE_OK ||
        sqlite3_prepare_v2(dbHandle, sql_select_id, -1, &stmt_select_id, 0) != SQLITE_OK ||
        sqlite3_prepare_v2(dbHandle, sql_insert_project, -1, &stmt_insert_project, 0) != SQLITE_OK) {
        std::cerr << "Error: [LogRepository] Failed to prepare statements: " << sqlite3_errmsg(dbHandle) << std::endl;
        database.rollback();
        return false;
    }

    for (const auto& daily : processedData) {
        // 1. 插入日期
        sqlite3_bind_text(stmt_insert_date, 1, daily.date.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt_insert_date) != SQLITE_DONE) {
            std::cerr << "Error: [LogRepository] Failed to execute date insert: " << sqlite3_errmsg(dbHandle) << std::endl;
            database.rollback();
            sqlite3_finalize(stmt_insert_date);
            sqlite3_finalize(stmt_select_id);
            sqlite3_finalize(stmt_insert_project);
            return false;
        }
        sqlite3_reset(stmt_insert_date); // 重置语句以便下次循环使用

        // 2. 获取 daily_log_id
        sqlite3_bind_text(stmt_select_id, 1, daily.date.c_str(), -1, SQLITE_TRANSIENT);
        long long daily_log_id = -1;
        if (sqlite3_step(stmt_select_id) == SQLITE_ROW) {
            daily_log_id = sqlite3_column_int64(stmt_select_id, 0);
        }
        sqlite3_reset(stmt_select_id);

        if (daily_log_id == -1) {
            std::cerr << "Error: [LogRepository] Could not find or insert daily log id for date: " << daily.date << std::endl;
            database.rollback();
            sqlite3_finalize(stmt_insert_date);
            sqlite3_finalize(stmt_select_id);
            sqlite3_finalize(stmt_insert_project);
            return false;
        }

        // 3. 插入所有项目
        for (const auto& project : daily.projects) {
            std::stringstream reps_ss;
            for (size_t i = 0; i < project.reps.size(); ++i) {
                reps_ss << project.reps[i] << (i < project.reps.size() - 1 ? "," : "");
            }
            std::string reps_list_str = reps_ss.str();

            sqlite3_bind_int64(stmt_insert_project, 1, daily_log_id);
            sqlite3_bind_text(stmt_insert_project, 2, project.projectName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt_insert_project, 3, project.weight);
            sqlite3_bind_text(stmt_insert_project, 4, reps_list_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt_insert_project, 5, project.volume);

            if (sqlite3_step(stmt_insert_project) != SQLITE_DONE) {
                std::cerr << "Error: [LogRepository] Failed to insert project: " << sqlite3_errmsg(dbHandle) << std::endl;
                database.rollback();
                sqlite3_finalize(stmt_insert_date);
                sqlite3_finalize(stmt_select_id);
                sqlite3_finalize(stmt_insert_project);
                return false;
            }
            sqlite3_reset(stmt_insert_project);
        }
    }

    // 清理所有语句
    sqlite3_finalize(stmt_insert_date);
    sqlite3_finalize(stmt_select_id);
    sqlite3_finalize(stmt_insert_project);

    if (!database.commit()) {
        std::cerr << "Error: [LogRepository] Failed to commit transaction." << std::endl;
        return false;
    }

    return true;
}