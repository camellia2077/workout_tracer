// src/db/DbInsertor.hpp

#ifndef DB_INSERTOR_H
#define DB_INSERTOR_H

#include "nlohmann/json.hpp"
#include "sqlite3.h"
#include <string>

class DbInsertor {
public:
    /**
     * @brief 将解析后的JSON数据插入到SQLite数据库中。
     * * @param db sqlite3数据库连接的指针。
     * @param jsonData 从reprocessed_json文件中读取并解析的nlohmann::json对象。
     * @return 如果插入成功返回true，否则返回false。
     */
    static bool insert_data(sqlite3* db, const nlohmann::json& jsonData);
};

#endif // DB_INSERTOR_H