#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <string>
#include <vector>
#include "common/parsed_data.h" // <-- 引用共享的数据结构

struct sqlite3; // 前向声明

class SqliteManager {
public:
    SqliteManager();
    ~SqliteManager();

    SqliteManager(const SqliteManager&) = delete;
    SqliteManager& operator=(const SqliteManager&) = delete;

    bool connect(const std::string& dbPath);
    void disconnect();
    bool saveData(const std::vector<DailyData>& processedData);

private:
    bool initializeSchema();
    sqlite3* db {nullptr};
};

#endif // SQLITE_MANAGER_H