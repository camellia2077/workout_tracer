// src/database/DatabaseManager.hpp

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "sqlite3.h"
#include <string>
#include <vector>
#include <map>

// LogEntry 结构体保持不变
struct LogEntry {
    std::string date;
    std::string exercise_name;
    std::vector<int> reps;
};

// [NEW] 新增结构体，用于封装单个周期的所有数据
struct CycleData {
    std::string type;
    int total_days;
    std::vector<LogEntry> logs;
};

class DatabaseManager {
public:
    /**
     * @brief 从数据库查询所有的训练日志，并按 cycle_id 分组。
     * @param db sqlite3数据库连接的指针。
     * @return 按 cycle_id 分组的日志数据。
     */
    // [MODIFIED] 更改了函数返回值
    static std::map<std::string, CycleData> query_all_logs(sqlite3* db);
};

#endif // DATABASE_MANAGER_H