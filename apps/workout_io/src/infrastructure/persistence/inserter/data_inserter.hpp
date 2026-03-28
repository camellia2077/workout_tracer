// db/inserter/data_inserter.hpp

#ifndef DB_INSERTER_DATA_INSERTER_HPP_
#define DB_INSERTER_DATA_INSERTER_HPP_

#include "domain/models/workout_item.hpp"
#include "sqlite3.h"
#include <vector>

class DataInserter {
public:
  explicit DataInserter(sqlite3* db_handle);

  /**
   * @brief 插入训练数据到数据库。
   * @param data 训练数据向量。
   * @return 成功返回 true，否则返回 false。
   */
  auto Insert(const std::vector<DailyData>& data) -> bool;

private:
  static constexpr int kColLogCycleId = 1;
  static constexpr int kColLogTotalDays = 2;
  static constexpr int kColLogDate = 3;
  static constexpr int kColLogDailyNote = 4;
  static constexpr int kColLogProjectNote = 5;
  static constexpr int kColLogExerciseName = 6;
  static constexpr int kColLogExerciseType = 7;
  static constexpr int kColLogTotalVolume = 8;

  static constexpr int kColSetLogId = 1;
  static constexpr int kColSetNumber = 2;
  static constexpr int kColSetWeight = 3;
  static constexpr int kColSetReps = 4;
  static constexpr int kColSetVolume = 5;
  static constexpr int kColSetUnit = 6;
  static constexpr int kColSetElasticWeight = 7;
  static constexpr int kColSetNote = 8;

  sqlite3* db_;
  
  auto InsertSets(sqlite3_stmt* stmt_set, sqlite3_int64 log_id,
                  const std::vector<SetData>& sets) -> void;
};

#endif // DB_INSERTER_DATA_INSERTER_HPP_