// report/database/database_manager.hpp

#ifndef REPORT_DATABASE_DATABASE_MANAGER_HPP_
#define REPORT_DATABASE_DATABASE_MANAGER_HPP_

#include "sqlite3.h"
#include <map>
#include <string>
#include <vector>

// 定义每一组的详细数据
struct SetDetail {
  int reps_;
  double weight_;
  std::string unit_;
  double elastic_band_weight_;
  std::string note_;
};

struct LogEntry {
  std::string date_;
  std::string daily_note_;
  std::string project_note_;
  std::string exercise_name_;
  std::string exercise_type_;
  std::vector<SetDetail> sets_;
};

struct CycleData {
  std::string type_;
  int total_days_;
  double average_intensity_ = 0.0;
  int session_count_ = 0;
  double vol_power_ = 0.0;
  double vol_hypertrophy_ = 0.0;
  double vol_endurance_ = 0.0;
  double total_volume_ = 0.0;
  std::vector<LogEntry> logs_;
};

struct PRRecord {
  std::string exercise_name_;
  double max_weight_;
  int reps_;
  std::string date_;
  double estimated_1rm_epley_;
  double estimated_1rm_brzycki_;
};

class DatabaseManager {
public:
  /**
   * @brief 从数据库查询所有的训练日志，并按 cycle_id 分组。
   * @param db sqlite3数据库连接的指针。
   * @return 按 cycle_id 分组的日志数据。
   */
  static auto QueryAllLogs(sqlite3* sqlite_db) -> std::map<std::string, CycleData>;
  /**
   * @brief 查询所有动作的个人记录 (PR)。
   */
  static auto QueryPRSummary(sqlite3* sqlite_db) -> std::vector<PRRecord>;
};

#endif // REPORT_DATABASE_DATABASE_MANAGER_HPP_
