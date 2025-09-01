// src/db/DbInsertor.cpp

#include "DbInsertor.hpp"
#include <iostream>

bool DbInsertor::insert_data(sqlite3* db, const nlohmann::json& jsonData) {
    char* zErrMsg = nullptr;

    // --- 1. 开始数据库事务 ---
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error starting transaction: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    // --- 2. 准备SQL插入语句 (使用预处理语句防止SQL注入) ---
    const char* sql_insert_log = "INSERT INTO training_logs (date, exercise_name, exercise_type, total_volume) VALUES (?, ?, ?, ?);";
    const char* sql_insert_set = "INSERT INTO training_sets (log_id, set_number, weight, reps, volume) VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt_log, *stmt_set;
    if (sqlite3_prepare_v2(db, sql_insert_log, -1, &stmt_log, NULL) != SQLITE_OK ||
        sqlite3_prepare_v2(db, sql_insert_set, -1, &stmt_set, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL); // 准备失败，回滚事务
        return false;
    }

    // --- 3. 解析JSON并绑定数据 ---
    try {
        const std::string exercise_type = jsonData.at("type").get<std::string>();

        for (const auto& session : jsonData.at("sessions")) {
            const std::string date = session.at("date").get<std::string>();

            for (const auto& exercise : session.at("exercises")) {
                const std::string exercise_name = exercise.at("name").get<std::string>();
                const double total_volume = exercise.at("totalVolume").get<double>();

                // 绑定参数到 training_logs 语句
                sqlite3_bind_text(stmt_log, 1, date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 2, exercise_name.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 3, exercise_type.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_double(stmt_log, 4, total_volume);

                // 执行插入并检查结果
                if (sqlite3_step(stmt_log) != SQLITE_DONE) {
                    std::cerr << "Error inserting training log: " << sqlite3_errmsg(db) << std::endl;
                    throw std::runtime_error("Log insertion failed.");
                }

                // 获取刚刚插入的 training_logs 的ID
                sqlite3_int64 last_log_id = sqlite3_last_insert_rowid(db);
                sqlite3_reset(stmt_log); // 重置语句以便下次使用

                // 遍历并插入每个set
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
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL); // 解析失败，回滚事务
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        return false;
    } catch (const std::runtime_error& e) {
        // 捕获由SQL插入失败抛出的异常
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL); // 插入失败，回滚事务
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        return false;
    }

    // --- 4. 最终化语句并提交事务 ---
    sqlite3_finalize(stmt_log);
    sqlite3_finalize(stmt_set);
    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error committing transaction: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}