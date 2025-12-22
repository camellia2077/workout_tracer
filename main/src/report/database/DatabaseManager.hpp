// report/database/DatabaseManager.hpp

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "sqlite3.h"
#include <string>
#include <vector>
#include <map>

// [NEW] 定义每一组的详细数据
struct SetDetail {
    int reps;
    double weight;
    std::string unit;
    double elastic_band_weight;
};

// [MODIFIED] 更新 LogEntry 以包含 SetDetail 列表
struct LogEntry {
    std::string date;
    std::string exercise_name;
    std::vector<SetDetail> sets; // 原来是 vector<int> reps
};

// CycleData 结构体保持不变
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
    static std::map<std::string, CycleData> query_all_logs(sqlite3* db);
};

#endif // DATABASE_MANAGER_H