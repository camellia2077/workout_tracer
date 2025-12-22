// db/facade/DbFacade.hpp

#ifndef DB_FACADE_H
#define DB_FACADE_H

#include <cjson/cJSON.h> // [MODIFIED] 引入 cJSON
#include "sqlite3.h"

/**
 * @brief 数据库操作的外观类。
 */
class DbFacade {
public:
    /**
     * @brief 将训练数据插入数据库。
     * @param db 数据库连接指针。
     * @param jsonData cJSON 对象指针 (const cJSON*)。
     * @return 成功返回true，失败返回false。
     */
    static bool insertTrainingData(sqlite3* db, const cJSON* jsonData);
};

#endif // DB_FACADE_H