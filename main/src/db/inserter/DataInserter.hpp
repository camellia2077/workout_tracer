// db/inserter/DataInserter.hpp

#ifndef TRAINING_DATA_INSERTER_H
#define TRAINING_DATA_INSERTER_H

// [MODIFIED] 移除 nlohmann_json，引入 cJSON
#include <cjson/cJSON.h> 
#include "sqlite3.h"

/**
 * @brief 负责将训练数据的JSON对象插入数据库。
 * 此类封装了SQL语句准备、绑定和执行的具体细节，
 * 但不负责事务管理。
 */
class DataInserter {
public:
    explicit DataInserter(sqlite3* db);

    /**
     * @brief 执行插入操作。
     * @param jsonData 包含训练数据的 cJSON 指针。
     * @return 如果成功返回true，如果发生任何错误则抛出std::runtime_error。
     */
    // [MODIFIED] 参数类型更新为 const cJSON*
    bool insert(const cJSON* jsonData);

private:
    sqlite3* db_;

    // [MODIFIED] 参数类型更新为 const cJSON*
    // 这里的 stmt 是已经准备好的 SQL 语句对象
    void insertSets(sqlite3_stmt* stmt, sqlite3_int64 logId, const cJSON* setsJson);
};

#endif // TRAINING_DATA_INSERTER_H