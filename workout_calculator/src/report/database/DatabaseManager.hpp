// src/database/DatabaseManager.hpp

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "sqlite3.h"
#include <string>
#include <vector>
#include <map>

// 定义 LogEntry 结构体，以便在不同模块间共享
struct LogEntry {
    std::string date;
    std::string exercise_name;
    std::vector<int> reps;
};

class DatabaseManager {
public:
    /**
     * @brief 从数据库查询所有的训练日志。
     * @param db sqlite3数据库连接的指针。
     * @return 按类型分组的日志数据。
     */
    static std::map<std::string, std::vector<LogEntry>> query_all_logs(sqlite3* db);
};

#endif // DATABASE_MANAGER_H