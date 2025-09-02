// src/db/DbInsertor.cpp

#include "DbInsertor.hpp"
#include <iostream>

bool DbInsertor::insert_data(sqlite3* db, const nlohmann::json& jsonData) {
    char* zErrMsg = nullptr;

    if (sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error starting transaction: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    // [MODIFIED] 更新了 INSERT 语句以包含新字段
    const char* sql_insert_log = "INSERT INTO training_logs (cycle_id, total_days, date, exercise_name, exercise_type, total_volume) VALUES (?, ?, ?, ?, ?, ?);";
    const char* sql_insert_set = "INSERT INTO training_sets (log_id, set_number, weight, reps, volume) VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt_log, *stmt_set;
    if (sqlite3_prepare_v2(db, sql_insert_log, -1, &stmt_log, NULL) != SQLITE_OK ||
        sqlite3_prepare_v2(db, sql_insert_set, -1, &stmt_set, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }

    try {
        // [MODIFIED] 从新的JSON结构中读取顶层数据
        const std::string cycle_id = jsonData.at("cycle_id").get<std::string>();
        const int total_days = jsonData.at("total_days").get<int>();
        const std::string exercise_type = jsonData.at("type").get<std::string>();

        for (const auto& session : jsonData.at("sessions")) {
            const std::string date = session.at("date").get<std::string>();

            for (const auto& exercise : session.at("exercises")) {
                const std::string exercise_name = exercise.at("name").get<std::string>();
                const double total_volume = exercise.at("totalVolume").get<double>();

                // [MODIFIED] 绑定新增的参数
                sqlite3_bind_text(stmt_log, 1, cycle_id.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt_log, 2, total_days);
                sqlite3_bind_text(stmt_log, 3, date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 4, exercise_name.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 5, exercise_type.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_double(stmt_log, 6, total_volume);

                if (sqlite3_step(stmt_log) != SQLITE_DONE) {
                    std::cerr << "Error inserting training log: " << sqlite3_errmsg(db) << std::endl;
                    throw std::runtime_error("Log insertion failed.");
                }

                sqlite3_int64 last_log_id = sqlite3_last_insert_rowid(db);
                sqlite3_reset(stmt_log);

                for (const auto& set : exercise.at("sets")) {
                    sqlite3_bind_int64(stmt_set, 1, last_log_id);
                    sqlite3_bind_int(stmt_set, 2, set.at("set").get<int>());
                    sqlite3_bind_double(stmt_set, 3, set.at("weight").get<double>());
                    sqlite3_bind_int(stmt_set, 4, set.at("reps").get<int>());
                    sqlite3_bind_double(stmt_set, 5, set.at("volume").get<double>());
                    
                    if (sqlite3_step(stmt_set) != SQLITE_DONE) {
                        std::cerr << "Error inserting training set: " << sqlite3_errmsg(db) << std::endl;
                        throw std::runtime_error("Set insertion failed.");
                    }
                    sqlite3_reset(stmt_set);
                }
            }
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        return false;
    } catch (const std::runtime_error& e) {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        return false;
    }

    sqlite3_finalize(stmt_log);
    sqlite3_finalize(stmt_set);
    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error committing transaction: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}