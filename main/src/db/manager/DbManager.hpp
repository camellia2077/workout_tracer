// db/manager/DbManager.hpp

#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include "sqlite3.h"

class DbManager {
public:
    DbManager(const std::string& dbPath);
    ~DbManager();

    // 禁止拷贝和赋值
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    bool open();
    void close();
    sqlite3* getConnection() const;

private:
    std::string dbPath_;
    sqlite3* db_ = nullptr;
    bool createTables();
};

#endif // DB_MANAGER_H