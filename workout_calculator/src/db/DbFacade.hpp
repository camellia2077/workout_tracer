// src/db/DbFacade.hpp

#ifndef DB_FACADE_H
#define DB_FACADE_H

#include "nlohmann/json.hpp"
#include "sqlite3.h"

/**
 * @brief 数据库操作的外观类。
 * 为客户端提供一个简单的接口来执行复杂操作，如数据插入。
 * 此类负责管理数据库事务。
 */
class DbFacade {
public:
    /**
     * @brief 将训练数据插入数据库。
     * 此方法会自动处理事务的开始、提交或回滚。
     * @param db 数据库连接指针。
     * @param jsonData 包含训练数据的JSON对象。
     * @return 成功返回true，失败返回false。
     */
    static bool insertTrainingData(sqlite3* db, const nlohmann::json& jsonData);
};

#endif // DB_FACADE_H