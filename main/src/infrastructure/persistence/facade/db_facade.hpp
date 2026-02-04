// db/facade/db_facade.hpp

#ifndef DB_FACADE_DB_FACADE_HPP_
#define DB_FACADE_DB_FACADE_HPP_

#include "domain/models/workout_item.hpp"
#include "sqlite3.h"
#include <vector>

/**
 * @brief 数据库操作的外观类。
 */
class DbFacade {
public:
  /**
   * @brief 将训练数据插入数据库。
   * @param db 数据库连接指针。
   * @param data 训练数据向量。
   * @return 成功返回 true，失败返回 false。
   */
  static auto InsertTrainingData(sqlite3* db,
                                 const std::vector<DailyData>& data) -> bool;
};

#endif // DB_FACADE_DB_FACADE_HPP_