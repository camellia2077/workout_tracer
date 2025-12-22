// db/inserter/DataInserter.cpp

#include "db/inserter/DataInserter.hpp"
#include <iostream>
#include <stdexcept>

DataInserter::DataInserter(sqlite3* db) : db_(db) {}

// [NEW] 实现辅助函数，处理具体的 Set 数据绑定和执行
void DataInserter::insertSets(sqlite3_stmt* stmt_set, sqlite3_int64 logId, const nlohmann::json& setsJson) {
    for (const auto& set : setsJson) {
        // 准备变量并处理可选字段
        double weight = 0.0;
        double elastic_band_weight = 0.0;
        std::string unit = "kg"; // 默认为 kg

        if (set.contains("weight")) weight = set.at("weight").get<double>();
        if (set.contains("elastic_band")) elastic_band_weight = set.at("elastic_band").get<double>();
        if (set.contains("unit")) unit = set.at("unit").get<std::string>();

        // 绑定参数
        // 注意：这里的索引需与 SQL 语句对应
        sqlite3_bind_int64(stmt_set, 1, logId);
        sqlite3_bind_int(stmt_set, 2, set.at("set").get<int>());
        sqlite3_bind_double(stmt_set, 3, weight);
        sqlite3_bind_int(stmt_set, 4, set.at("reps").get<int>());
        sqlite3_bind_double(stmt_set, 5, set.at("volume").get<double>());
        sqlite3_bind_text(stmt_set, 6, unit.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt_set, 7, elastic_band_weight);
        
        if (sqlite3_step(stmt_set) != SQLITE_DONE) {
                throw std::runtime_error("Error inserting training set: " + std::string(sqlite3_errmsg(db_)));
        }
        sqlite3_reset(stmt_set);
    }
}

bool DataInserter::insert(const nlohmann::json& jsonData) {
    const char* sql_insert_log = "INSERT INTO training_logs (cycle_id, total_days, date, exercise_name, exercise_type, total_volume) VALUES (?, ?, ?, ?, ?, ?);";
    const char* sql_insert_set = "INSERT INTO training_sets (log_id, set_number, weight, reps, volume, unit, elastic_band_weight) VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt_log = nullptr;
    sqlite3_stmt *stmt_set = nullptr;

    // 准备语句
    if (sqlite3_prepare_v2(db_, sql_insert_log, -1, &stmt_log, NULL) != SQLITE_OK ||
        sqlite3_prepare_v2(db_, sql_insert_set, -1, &stmt_set, NULL) != SQLITE_OK) {
        std::string errMsg = "Failed to prepare statement: ";
        errMsg += sqlite3_errmsg(db_);
        if (stmt_log) sqlite3_finalize(stmt_log);
        if (stmt_set) sqlite3_finalize(stmt_set);
        throw std::runtime_error(errMsg);
    }

    try {
        const std::string cycle_id = jsonData.at("cycle_id").get<std::string>();
        const int total_days = jsonData.at("total_days").get<int>();
        const std::string exercise_type = jsonData.at("type").get<std::string>();

        // 第 1 层循环：遍历 Session
        for (const auto& session : jsonData.at("sessions")) {
            const std::string date = session.at("date").get<std::string>();

            // 第 2 层循环：遍历 Exercise
            for (const auto& exercise : session.at("exercises")) {
                // 插入 training_logs
                sqlite3_bind_text(stmt_log, 1, cycle_id.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt_log, 2, total_days);
                sqlite3_bind_text(stmt_log, 3, date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 4, exercise.at("name").get<std::string>().c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 5, exercise_type.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_double(stmt_log, 6, exercise.at("totalVolume").get<double>());

                if (sqlite3_step(stmt_log) != SQLITE_DONE) {
                    throw std::runtime_error("Error inserting training log: " + std::string(sqlite3_errmsg(db_)));
                }
                sqlite3_reset(stmt_log);

                // 获取刚插入的 log_id
                sqlite3_int64 last_log_id = sqlite3_last_insert_rowid(db_);

                // [MODIFIED] 调用辅助函数插入 Sets，消除了原本的第 3 层嵌套循环
                insertSets(stmt_set, last_log_id, exercise.at("sets"));
            }
        }
    } catch (const std::exception& e) {
        // 确保在抛出异常前释放资源
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        throw; // 重新抛出异常
    }

    // 成功完成所有操作后，释放资源
    sqlite3_finalize(stmt_log);
    sqlite3_finalize(stmt_set);
    return true;
}