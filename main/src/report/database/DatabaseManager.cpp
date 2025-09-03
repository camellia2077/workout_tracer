// src/database/DatabaseManager.cpp

#include "DatabaseManager.hpp"
#include <iostream>

// [MODIFIED] 完全重写了此函数以按 cycle_id 分组
std::map<std::string, CycleData> DatabaseManager::query_all_logs(sqlite3* db) {
    std::cout << "Querying data from database..." << std::endl;
    std::map<std::string, CycleData> data_by_cycle;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT l.cycle_id, l.total_days, l.exercise_type, l.id, l.date, l.exercise_name, s.reps "
                      "FROM training_logs l "
                      "JOIN training_sets s ON l.id = s.log_id "
                      "ORDER BY l.cycle_id, l.date, l.id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare query statement: " << sqlite3_errmsg(db) << std::endl;
        return data_by_cycle;
    }

    // 临时存储每个 log_id 对应的 LogEntry
    std::map<long long, LogEntry> temp_entries;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string cycle_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int total_days = sqlite3_column_int(stmt, 1);
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        long long log_id = sqlite3_column_int64(stmt, 3);

        // 如果这是这个 cycle_id 的第一条记录，初始化 CycleData
        if (data_by_cycle.find(cycle_id) == data_by_cycle.end()) {
            data_by_cycle[cycle_id].total_days = total_days;
            data_by_cycle[cycle_id].type = type;
        }

        // 如果这是这个 log_id 的第一条记录，创建 LogEntry
        if (temp_entries.find(log_id) == temp_entries.end()) {
            LogEntry entry;
            entry.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            entry.exercise_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            temp_entries[log_id] = entry;
        }
        
        // 将次数添加到对应的 LogEntry 中
        temp_entries[log_id].reps.push_back(sqlite3_column_int(stmt, 6));
    }

    // 将处理完的 LogEntry 按 cycle_id 归类
    // 重新查询一次以确保 log_id 和 cycle_id 的正确映射
    if (sqlite3_prepare_v2(db, "SELECT id, cycle_id FROM training_logs", -1, &stmt, NULL) == SQLITE_OK) {
        std::map<long long, std::string> log_to_cycle_map;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            log_to_cycle_map[sqlite3_column_int64(stmt, 0)] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        }

        for (const auto& [log_id, entry] : temp_entries) {
            const std::string& cycle_id = log_to_cycle_map[log_id];
            data_by_cycle[cycle_id].logs.push_back(entry);
        }
    }


    sqlite3_finalize(stmt);
    std::cout << "Data queried successfully." << std::endl;
    return data_by_cycle;
}