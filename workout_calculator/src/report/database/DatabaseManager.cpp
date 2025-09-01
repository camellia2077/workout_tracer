// src/database/DatabaseManager.cpp

#include "DatabaseManager.hpp"
#include <iostream>

std::map<std::string, std::vector<LogEntry>> DatabaseManager::query_all_logs(sqlite3* db) {
    std::cout << "Querying data from database..." << std::endl;
    std::map<std::string, std::vector<LogEntry>> data_by_type;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT l.id, l.date, l.exercise_name, l.exercise_type, s.reps "
                      "FROM training_logs l "
                      "JOIN training_sets s ON l.id = s.log_id "
                      "ORDER BY l.date, l.id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare query statement: " << sqlite3_errmsg(db) << std::endl;
        return data_by_type;
    }

    std::map<long long, LogEntry> temp_logs;
    std::map<long long, std::string> log_id_to_type;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long log_id = sqlite3_column_int64(stmt, 0);
        
        if (temp_logs.find(log_id) == temp_logs.end()) {
            LogEntry entry;
            entry.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            entry.exercise_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            temp_logs[log_id] = entry;
            log_id_to_type[log_id] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        }
        
        temp_logs[log_id].reps.push_back(sqlite3_column_int(stmt, 4));
    }

    for(auto const& [log_id, entry] : temp_logs) {
        const std::string& type = log_id_to_type[log_id];
        data_by_type[type].push_back(entry);
    }

    sqlite3_finalize(stmt);
    std::cout << "Data queried successfully." << std::endl;
    return data_by_type;
}