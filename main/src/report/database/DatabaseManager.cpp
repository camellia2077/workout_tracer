
// report/database/DatabaseManager.cpp

#include "DatabaseManager.hpp"
#include <iostream>

std::map<std::string, CycleData> DatabaseManager::query_all_logs(sqlite3* db) {
    std::cout << "Querying data from database..." << std::endl;
    std::map<std::string, CycleData> data_by_cycle;
    sqlite3_stmt* stmt;

    // SQL 查询已经包含了 exercise_type (l.exercise_type 是第 3 列，索引为 2)
    const char* sql = "SELECT l.cycle_id, l.total_days, l.exercise_type, l.id, l.date, l.exercise_name, "
                      "s.reps, s.weight, s.unit, s.elastic_band_weight "
                      "FROM training_logs l "
                      "JOIN training_sets s ON l.id = s.log_id "
                      "ORDER BY l.cycle_id, l.date, l.id, s.set_number;"; 

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare query statement: " << sqlite3_errmsg(db) << std::endl;
        return data_by_cycle;
    }

    // 临时存储每个 log_id 对应的 LogEntry
    std::map<long long, LogEntry> temp_entries;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string cycle_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int total_days = sqlite3_column_int(stmt, 1);
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)); // 获取类型
        long long log_id = sqlite3_column_int64(stmt, 3);

        // 初始化 CycleData
        if (data_by_cycle.find(cycle_id) == data_by_cycle.end()) {
            data_by_cycle[cycle_id].total_days = total_days;
            // 注意：因为现在一个 Cycle 包含混合类型，这里的 type 可能会是该 Cycle 中第一条记录的类型
            // 但这不影响后续 LogEntry 的准确性
            data_by_cycle[cycle_id].type = type; 
        }

        // 初始化 LogEntry
        if (temp_entries.find(log_id) == temp_entries.end()) {
            LogEntry entry;
            entry.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            entry.exercise_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            entry.exercise_type = type; // [NEW] 将类型赋值给具体的动作条目
            temp_entries[log_id] = entry;
        }
        
        // 提取详细的组信息
        SetDetail detail;
        detail.reps = sqlite3_column_int(stmt, 6);
        detail.weight = sqlite3_column_double(stmt, 7);
        
        const unsigned char* unit_text = sqlite3_column_text(stmt, 8);
        detail.unit = unit_text ? reinterpret_cast<const char*>(unit_text) : "kg";

        detail.elastic_band_weight = sqlite3_column_double(stmt, 9);

        // 将组信息添加到对应的 LogEntry 中
        temp_entries[log_id].sets.push_back(detail);
    }

    // 将处理完的 LogEntry 按 cycle_id 归类
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
