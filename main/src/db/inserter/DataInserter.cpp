// db/inserter/DataInserter.cpp

#include "db/inserter/DataInserter.hpp"
#include "common/CJsonHelper.hpp" // [NEW] 引入 cJSON 辅助头文件
#include <iostream>
#include <stdexcept>

// [NEW] 静态辅助函数：安全获取 Double 值
static double get_double(const cJSON* item, const char* key, double defaultVal = 0.0) {
    cJSON* obj = cJSON_GetObjectItemCaseSensitive(item, key);
    if (cJSON_IsNumber(obj)) {
        return obj->valuedouble;
    }
    return defaultVal;
}

// [NEW] 静态辅助函数：安全获取 Int 值
static int get_int(const cJSON* item, const char* key, int defaultVal = 0) {
    cJSON* obj = cJSON_GetObjectItemCaseSensitive(item, key);
    if (cJSON_IsNumber(obj)) {
        return obj->valueint;
    }
    return defaultVal;
}

// [NEW] 静态辅助函数：安全获取 String 值
static std::string get_string(const cJSON* item, const char* key, const std::string& defaultVal = "") {
    cJSON* obj = cJSON_GetObjectItemCaseSensitive(item, key);
    if (cJSON_IsString(obj) && (obj->valuestring != nullptr)) {
        return std::string(obj->valuestring);
    }
    return defaultVal;
}

DataInserter::DataInserter(sqlite3* db) : db_(db) {}

// [MODIFIED] 辅助函数现在接收 cJSON 指针
void DataInserter::insertSets(sqlite3_stmt* stmt_set, sqlite3_int64 logId, const cJSON* setsArray) {
    if (!setsArray || !cJSON_IsArray(setsArray)) return;

    cJSON* setItem = nullptr;
    // 使用 cJSON 宏遍历数组
    cJSON_ArrayForEach(setItem, setsArray) {
        // 准备变量并处理可选字段
        double weight = get_double(setItem, "weight", 0.0);
        double elastic_band_weight = get_double(setItem, "elastic_band", 0.0);
        std::string unit = get_string(setItem, "unit", "kg");

        // 绑定参数
        sqlite3_bind_int64(stmt_set, 1, logId);
        sqlite3_bind_int(stmt_set, 2, get_int(setItem, "set"));
        sqlite3_bind_double(stmt_set, 3, weight);
        sqlite3_bind_int(stmt_set, 4, get_int(setItem, "reps"));
        sqlite3_bind_double(stmt_set, 5, get_double(setItem, "volume"));
        sqlite3_bind_text(stmt_set, 6, unit.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt_set, 7, elastic_band_weight);
        
        if (sqlite3_step(stmt_set) != SQLITE_DONE) {
            throw std::runtime_error("Error inserting training set: " + std::string(sqlite3_errmsg(db_)));
        }
        sqlite3_reset(stmt_set);
    }
}

// [MODIFIED] 主插入函数现在接收 cJSON 指针
bool DataInserter::insert(const cJSON* jsonData) {
    if (!jsonData) return false;

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
        std::string cycle_id = get_string(jsonData, "cycle_id");
        int total_days = get_int(jsonData, "total_days");
        
        // 获取 Sessions 数组
        cJSON* sessions = cJSON_GetObjectItemCaseSensitive(jsonData, "sessions");
        cJSON* session = nullptr;

        // 第 1 层循环：遍历 Session
        cJSON_ArrayForEach(session, sessions) {
            std::string date = get_string(session, "date");

            // 获取 Exercises 数组
            cJSON* exercises = cJSON_GetObjectItemCaseSensitive(session, "exercises");
            cJSON* exercise = nullptr;

            // 第 2 层循环：遍历 Exercise
            cJSON_ArrayForEach(exercise, exercises) {
                
                // 获取每个动作的具体类型
                std::string current_type = get_string(exercise, "type", "unknown");
                std::string name = get_string(exercise, "name");
                double totalVolume = get_double(exercise, "totalVolume");

                // 插入 training_logs
                sqlite3_bind_text(stmt_log, 1, cycle_id.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt_log, 2, total_days);
                sqlite3_bind_text(stmt_log, 3, date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 4, name.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_log, 5, current_type.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_double(stmt_log, 6, totalVolume);

                if (sqlite3_step(stmt_log) != SQLITE_DONE) {
                    throw std::runtime_error("Error inserting training log: " + std::string(sqlite3_errmsg(db_)));
                }
                sqlite3_reset(stmt_log);

                // 获取刚插入的 log_id
                sqlite3_int64 last_log_id = sqlite3_last_insert_rowid(db_);

                // 调用辅助函数插入 Sets (传入 sets 数组对象)
                insertSets(stmt_set, last_log_id, cJSON_GetObjectItemCaseSensitive(exercise, "sets"));
            }
        }
    } catch (const std::exception& e) {
        sqlite3_finalize(stmt_log);
        sqlite3_finalize(stmt_set);
        throw; 
    }

    sqlite3_finalize(stmt_log);
    sqlite3_finalize(stmt_set);
    return true;
}