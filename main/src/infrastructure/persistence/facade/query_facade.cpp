// infrastructure/persistence/facade/query_facade.cpp
#include "infrastructure/persistence/facade/query_facade.hpp"
#include <iostream>

auto QueryFacade::QueryAllPRs(sqlite3* db) -> std::vector<PersonalRecord> {
  std::vector<PersonalRecord> prs;
  const char* sql = 
      "SELECT l.exercise_name, MAX(s.weight), s.reps, l.date "
      "FROM training_sets s "
      "JOIN training_logs l ON s.log_id = l.id "
      "GROUP BY l.exercise_name "
      "ORDER BY l.exercise_name ASC;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing PR query: " << sqlite3_errmsg(db) << std::endl;
    return prs;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    PersonalRecord pr;
    pr.exercise_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    pr.max_weight = sqlite3_column_double(stmt, 1);
    pr.reps = sqlite3_column_int(stmt, 2);
    pr.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    // Calculate estimated 1RM
    if (pr.reps <= 1) {
      pr.estimated_1rm_epley = pr.max_weight;
      pr.estimated_1rm_brzycki = pr.max_weight;
    } else {
      pr.estimated_1rm_epley = pr.max_weight * (1.0 + static_cast<double>(pr.reps) / 30.0);
      double brzycki_denom = 37.0 - static_cast<double>(pr.reps);
      pr.estimated_1rm_brzycki = (brzycki_denom > 0) ? (pr.max_weight * (36.0 / brzycki_denom)) : (pr.max_weight * 36.0);
    }
    
    prs.push_back(pr);
  }

  sqlite3_finalize(stmt);
  return prs;
}

auto QueryFacade::GetExercisesByType(sqlite3* db, const std::string& type_filter) 
    -> std::vector<ExerciseInfo> {
  std::vector<ExerciseInfo> exercises;
  std::string sql = "SELECT DISTINCT exercise_name, exercise_type FROM training_logs";
  if (!type_filter.empty()) {
    sql += " WHERE exercise_type = ?";
  }
  sql += " ORDER BY exercise_name ASC;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing exercise list query: " << sqlite3_errmsg(db) << std::endl;
    return exercises;
  }

  if (!type_filter.empty()) {
    sqlite3_bind_text(stmt, 1, type_filter.c_str(), -1, SQLITE_STATIC);
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    ExerciseInfo info;
    info.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    info.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    exercises.push_back(info);
  }

  sqlite3_finalize(stmt);
  return exercises;
}

auto QueryFacade::GetAllCycles(sqlite3* db) -> std::vector<CycleRecord> {
  std::vector<CycleRecord> cycles;
  const char* sql = 
      "SELECT cycle_id, total_days, GROUP_CONCAT(DISTINCT exercise_type), MIN(date), MAX(date) "
      "FROM training_logs "
      "GROUP BY cycle_id "
      "ORDER BY MIN(date) DESC;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing cycles query: " << sqlite3_errmsg(db) << std::endl;
    return cycles;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    CycleRecord record;
    record.cycle_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    record.total_days = sqlite3_column_int(stmt, 1);
    
    const unsigned char* types_text = sqlite3_column_text(stmt, 2);
    record.type = (types_text != nullptr) ? reinterpret_cast<const char*>(types_text) : "";
    
    record.start_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    record.end_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    cycles.push_back(record);
  }

  sqlite3_finalize(stmt);
  return cycles;
}
auto QueryFacade::GetVolumeStats(sqlite3* db, const std::string& cycle_id, const std::string& type)
    -> std::optional<VolumeStats> {
  const char* sql = 
      "SELECT l.cycle_id, l.exercise_type, SUM(s.weight * s.reps), MAX(l.total_days), "
      "CAST(SUM(s.weight * s.reps) AS DOUBLE) / SUM(s.reps), "
      "COUNT(DISTINCT l.id), SUM(s.reps), COUNT(s.id), "
      "SUM(CASE WHEN s.reps BETWEEN 1 AND 5 THEN s.weight * s.reps ELSE 0 END), "
      "SUM(CASE WHEN s.reps BETWEEN 6 AND 12 THEN s.weight * s.reps ELSE 0 END), "
      "SUM(CASE WHEN s.reps >= 13 THEN s.weight * s.reps ELSE 0 END) "
      "FROM training_logs l "
      "JOIN training_sets s ON l.id = s.log_id "
      "WHERE l.cycle_id = ? AND l.exercise_type = ? "
      "GROUP BY l.cycle_id, l.exercise_type;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing advanced volume stats query: " << sqlite3_errmsg(db) << std::endl;
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, cycle_id.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);

  std::optional<VolumeStats> stats;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    VolumeStats v_stats;
    v_stats.cycle_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    v_stats.exercise_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    v_stats.total_volume = sqlite3_column_double(stmt, 2);
    v_stats.total_days = sqlite3_column_int(stmt, 3);
    v_stats.average_intensity = sqlite3_column_double(stmt, 4);
    v_stats.session_count = sqlite3_column_int(stmt, 5);
    v_stats.total_reps = sqlite3_column_int(stmt, 6);
    v_stats.total_sets = sqlite3_column_int(stmt, 7);
    v_stats.vol_power = sqlite3_column_double(stmt, 8);
    v_stats.vol_hypertrophy = sqlite3_column_double(stmt, 9);
    v_stats.vol_endurance = sqlite3_column_double(stmt, 10);
    stats = v_stats;
  }

  sqlite3_finalize(stmt);
  return stats;
}
