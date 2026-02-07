// infrastructure/persistence/facade/query_facade.cpp
#include "infrastructure/persistence/facade/query_facade.hpp"

#include <iostream>

auto QueryFacade::QueryAllPRs(sqlite3* sqlite_db)
    -> std::vector<PersonalRecord> {
  std::vector<PersonalRecord> prs;
  const char* sql =
      "SELECT l.exercise_name, MAX(s.weight), s.reps, l.date "
      "FROM training_sets s "
      "JOIN training_logs l ON s.log_id = l.id "
      "GROUP BY l.exercise_name "
      "ORDER BY l.exercise_name ASC;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing PR query: " << sqlite3_errmsg(sqlite_db)
              << std::endl;
    return prs;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    PersonalRecord record;
    record.exercise_name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    record.max_weight = sqlite3_column_double(stmt, 1);
    record.reps = sqlite3_column_int(stmt, 2);
    record.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    // Calculate estimated 1RM
    if (record.reps <= 1) {
      record.estimated_1rm_epley = record.max_weight;
      record.estimated_1rm_brzycki = record.max_weight;
    } else {
      constexpr double kEpleyDivisor = 30.0;
      constexpr double kBrzyckiMultiplier = 36.0;
      constexpr double kBrzyckiConstant = 37.0;

      record.estimated_1rm_epley =
          record.max_weight *
          (1.0 + static_cast<double>(record.reps) / kEpleyDivisor);
      double brzycki_denom =
          kBrzyckiConstant - static_cast<double>(record.reps);
      record.estimated_1rm_brzycki =
          (brzycki_denom > 0)
              ? (record.max_weight * (kBrzyckiMultiplier / brzycki_denom))
              : (record.max_weight * kBrzyckiMultiplier);
    }

    prs.push_back(record);
  }

  sqlite3_finalize(stmt);
  return prs;
}

auto QueryFacade::GetExercisesByType(sqlite3* sqlite_db,
                                     const std::string& type_filter)
    -> std::vector<ExerciseInfo> {
  std::vector<ExerciseInfo> exercises;
  std::string sql =
      "SELECT DISTINCT exercise_name, exercise_type FROM training_logs";
  if (!type_filter.empty()) {
    sql += " WHERE exercise_type = ?";
  }
  sql += " ORDER BY exercise_name ASC;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(sqlite_db, sql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    std::cerr << "SQL error preparing exercise list query: "
              << sqlite3_errmsg(sqlite_db) << std::endl;
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

auto QueryFacade::GetAllCycles(sqlite3* sqlite_db) -> std::vector<CycleRecord> {
  std::vector<CycleRecord> cycles;
  const char* sql =
      "SELECT cycle_id, total_days, GROUP_CONCAT(DISTINCT exercise_type), "
      "MIN(date), MAX(date) "
      "FROM training_logs "
      "GROUP BY cycle_id "
      "ORDER BY MIN(date) DESC;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing cycles query: "
              << sqlite3_errmsg(sqlite_db) << std::endl;
    return cycles;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    CycleRecord record;
    record.cycle_id =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    record.total_days = sqlite3_column_int(stmt, 1);

    const unsigned char* types_text = sqlite3_column_text(stmt, 2);
    record.type = (types_text != nullptr)
                      ? reinterpret_cast<const char*>(types_text)
                      : "";

    record.start_date =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    record.end_date =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    cycles.push_back(record);
  }

  sqlite3_finalize(stmt);
  return cycles;
}
auto QueryFacade::GetVolumeStats(sqlite3* sqlite_db,
                                 const std::string& cycle_id,
                                 const std::string& type)
    -> std::optional<VolumeStats> {
  const char* sql =
      "SELECT l.cycle_id, l.exercise_type, SUM(s.weight * s.reps), "
      "MAX(l.total_days), "
      "CAST(SUM(s.weight * s.reps) AS DOUBLE) / SUM(s.reps), "
      "COUNT(DISTINCT l.id), SUM(s.reps), COUNT(s.id), "
      "SUM(CASE WHEN s.reps BETWEEN 1 AND 5 THEN s.weight * s.reps ELSE 0 "
      "END), "
      "SUM(CASE WHEN s.reps BETWEEN 6 AND 12 THEN s.weight * s.reps ELSE 0 "
      "END), "
      "SUM(CASE WHEN s.reps >= 13 THEN s.weight * s.reps ELSE 0 END) "
      "FROM training_logs l "
      "JOIN training_sets s ON l.id = s.log_id "
      "WHERE l.cycle_id = ? AND l.exercise_type = ? "
      "GROUP BY l.cycle_id, l.exercise_type;";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "SQL error preparing advanced volume stats query: "
              << sqlite3_errmsg(sqlite_db) << std::endl;
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, cycle_id.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);

  std::optional<VolumeStats> stats;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    constexpr int kColCycleId = 0;
    constexpr int kColExerciseType = 1;
    constexpr int kColTotalVolume = 2;
    constexpr int kColTotalDays = 3;
    constexpr int kColAvgIntensity = 4;
    constexpr int kColSessionCount = 5;
    constexpr int kColTotalReps = 6;
    constexpr int kColTotalSets = 7;
    constexpr int kColVolPower = 8;
    constexpr int kColVolHypertrophy = 9;
    constexpr int kColVolEndurance = 10;

    VolumeStats v_stats;
    v_stats.cycle_id =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColCycleId));
    v_stats.exercise_type = reinterpret_cast<const char*>(
        sqlite3_column_text(stmt, kColExerciseType));
    v_stats.total_volume = sqlite3_column_double(stmt, kColTotalVolume);
    v_stats.total_days = sqlite3_column_int(stmt, kColTotalDays);
    v_stats.average_intensity = sqlite3_column_double(stmt, kColAvgIntensity);
    v_stats.session_count = sqlite3_column_int(stmt, kColSessionCount);
    v_stats.total_reps = sqlite3_column_int(stmt, kColTotalReps);
    v_stats.total_sets = sqlite3_column_int(stmt, kColTotalSets);
    v_stats.vol_power = sqlite3_column_double(stmt, kColVolPower);
    v_stats.vol_hypertrophy = sqlite3_column_double(stmt, kColVolHypertrophy);
    v_stats.vol_endurance = sqlite3_column_double(stmt, kColVolEndurance);
    stats = v_stats;
  }

  sqlite3_finalize(stmt);
  return stats;
}
