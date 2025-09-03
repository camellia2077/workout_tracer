// src/db/TrainingDataInserter.cpp

#include "TrainingDataInserter.hpp"
#include <iostream>
#include <stdexcept>

TrainingDataInserter::TrainingDataInserter(sqlite3* db) : db_(db) {}

bool TrainingDataInserter::insert(const nlohmann::json& jsonData) {
    const char* sql_insert_log = "INSERT INTO training_logs (cycle_id, total_days, date, exercise_name, exercise_type, total_volume) VALUES (?, ?, ?, ?, ?, ?);";
    const char* sql_insert_set = "INSERT INTO training_sets (log_id, set_number, weight, reps, volume) VALUES (?, ?, ?, ?, ?);";
    
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

        for (const auto& session : jsonData.at("sessions")) {
            const std::string date = session.at("date").get<std::string>();

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

                // 插入 training_sets
                for (const auto& set : exercise.at("sets")) {
                    sqlite3_bind_int64(stmt_set, 1, last_log_id);
                    sqlite3_bind_int(stmt_set, 2, set.at("set").get<int>());
                    sqlite3_bind_double(stmt_set, 3, set.at("weight").get<double>());
                    sqlite3_bind_int(stmt_set, 4, set.at("reps").get<int>());
                    sqlite3_bind_double(stmt_set, 5, set.at("volume").get<double>());
                    
                    if (sqlite3_step(stmt_set) != SQLITE_DONE) {
                         throw std::runtime_error("Error inserting training set: " + std::string(sqlite3_errmsg(db_)));
                    }
                    sqlite3_reset(stmt_set);
                }
            }
        }
    } catch (const std::exception& e) {
        // 确保在抛出异常前释放资源
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        // 重新抛出，以便上层（Facade）可以捕获并回滚事务
        throw; 
    }

    // 成功完成所有操作后，释放资源
    sqlite3_finalize(stmt_log);
    sqlite3_finalize(stmt_set);
    return true;
}