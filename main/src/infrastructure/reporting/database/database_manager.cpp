// report/database/database_manager.cpp

#include "infrastructure/reporting/database/database_manager.hpp"

#include <iostream>

auto DatabaseManager::QueryAllLogs(sqlite3* sqlite_db)
    -> std::map<std::string, CycleData> {
  std::cout << "Querying data from database..." << std::endl;
  std::map<std::string, CycleData> data_by_cycle;
  sqlite3_stmt* stmt = nullptr;

  const char* sql =
      "SELECT l.cycle_id, l.total_days, l.exercise_type, l.id, "
      "l.date, l.exercise_name, l.daily_note, l.project_note, "
      "s.reps, s.weight, s.unit, s.elastic_band_weight, "
      "s.set_note "
      "FROM training_logs l "
      "JOIN training_sets s ON l.id = s.log_id "
      "ORDER BY l.cycle_id, l.date, l.id, s.set_number;";

  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Failed to prepare query statement: "
              << sqlite3_errmsg(sqlite_db) << std::endl;
    return data_by_cycle;
  }

  std::map<long long, LogEntry> temp_entries;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    constexpr int kColCycleId = 0;
    constexpr int kColTotalDays = 1;
    constexpr int kColType = 2;
    constexpr int kColLogId = 3;
    constexpr int kColDate = 4;
    constexpr int kColExerciseName = 5;
    constexpr int kColDailyNote = 6;
    constexpr int kColProjectNote = 7;
    constexpr int kColReps = 8;
    constexpr int kColWeight = 9;
    constexpr int kColUnit = 10;
    constexpr int kColElasticBandWeight = 11;
    constexpr int kColSetNote = 12;

    std::string cycle_id =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColCycleId));
    int total_days = sqlite3_column_int(stmt, kColTotalDays);
    std::string type =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColType));
    long long log_id = sqlite3_column_int64(stmt, kColLogId);

    if (!data_by_cycle.contains(cycle_id)) {
      data_by_cycle[cycle_id].total_days_ = total_days;
      data_by_cycle[cycle_id].type_ = type;
    }

    if (!temp_entries.contains(log_id)) {
      LogEntry entry;
      entry.date_ =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColDate));
      entry.exercise_name_ = reinterpret_cast<const char*>(
          sqlite3_column_text(stmt, kColExerciseName));
      const unsigned char* note_text = sqlite3_column_text(stmt, kColDailyNote);
      entry.daily_note_ = (note_text != nullptr)
                              ? reinterpret_cast<const char*>(note_text)
                              : "";
      const unsigned char* project_note_text =
          sqlite3_column_text(stmt, kColProjectNote);
      entry.project_note_ =
          (project_note_text != nullptr)
              ? reinterpret_cast<const char*>(project_note_text)
              : "";
      entry.exercise_type_ = type;
      temp_entries[log_id] = entry;
    }

    SetDetail detail;
    detail.reps_ = sqlite3_column_int(stmt, kColReps);
    detail.weight_ = sqlite3_column_double(stmt, kColWeight);

    const unsigned char* unit_text = sqlite3_column_text(stmt, kColUnit);
    detail.unit_ = (unit_text != nullptr)
                       ? reinterpret_cast<const char*>(unit_text)
                       : "kg";

    detail.elastic_band_weight_ =
        sqlite3_column_double(stmt, kColElasticBandWeight);
    const unsigned char* set_note_text = sqlite3_column_text(stmt, kColSetNote);
    detail.note_ = (set_note_text != nullptr)
                       ? reinterpret_cast<const char*>(set_note_text)
                       : "";

    temp_entries[log_id].sets_.push_back(detail);
  }

  if (sqlite3_prepare_v2(sqlite_db, "SELECT id, cycle_id FROM training_logs",
                         -1, &stmt, nullptr) == SQLITE_OK) {
    std::map<long long, std::string> log_to_cycle_map;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      log_to_cycle_map[sqlite3_column_int64(stmt, 0)] =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }

    for (const auto& [log_id, entry] : temp_entries) {
      if (log_to_cycle_map.contains(log_id)) {
        const std::string& cycle_id = log_to_cycle_map.at(log_id);
        data_by_cycle[cycle_id].logs_.push_back(entry);
      }
    }
  }
  sqlite3_finalize(stmt);

  // Fetch advanced metrics for each cycle
  for (auto& [cycle_id, cycle_data] : data_by_cycle) {
    const char* stats_sql =
        "SELECT SUM(s.weight * s.reps), "
        "CAST(SUM(s.weight * s.reps) AS DOUBLE) / SUM(s.reps), "
        "COUNT(DISTINCT l.id), "
        "SUM(CASE WHEN s.reps BETWEEN 1 AND 5 THEN s.weight * s.reps ELSE 0 "
        "END), "
        "SUM(CASE WHEN s.reps BETWEEN 6 AND 12 THEN s.weight * s.reps ELSE 0 "
        "END), "
        "SUM(CASE WHEN s.reps >= 13 THEN s.weight * s.reps ELSE 0 END) "
        "FROM training_logs l "
        "JOIN training_sets s ON l.id = s.log_id "
        "WHERE l.cycle_id = ? "
        "GROUP BY l.cycle_id;";

    if (sqlite3_prepare_v2(sqlite_db, stats_sql, -1, &stmt, nullptr) ==
        SQLITE_OK) {
      sqlite3_bind_text(stmt, 1, cycle_id.c_str(), -1, SQLITE_STATIC);
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        constexpr int kColTotalVolume = 0;
        constexpr int kColAvgIntensity = 1;
        constexpr int kColSessionCount = 2;
        constexpr int kColVolumePower = 3;
        constexpr int kColVolumeHypertrophy = 4;
        constexpr int kColVolumeEndurance = 5;

        cycle_data.total_volume_ = sqlite3_column_double(stmt, kColTotalVolume);
        cycle_data.average_intensity_ =
            sqlite3_column_double(stmt, kColAvgIntensity);
        cycle_data.session_count_ = sqlite3_column_int(stmt, kColSessionCount);
        cycle_data.vol_power_ = sqlite3_column_double(stmt, kColVolumePower);
        cycle_data.vol_hypertrophy_ =
            sqlite3_column_double(stmt, kColVolumeHypertrophy);
        cycle_data.vol_endurance_ =
            sqlite3_column_double(stmt, kColVolumeEndurance);
      }
      sqlite3_finalize(stmt);
    }
  }

  std::cout << "Data queried successfully." << std::endl;
  return data_by_cycle;
}

auto DatabaseManager::QueryPRSummary(sqlite3* sqlite_db)
    -> std::vector<PRRecord> {
  const char* sql =
      "SELECT exercise_name, MAX(weight), reps, date "
      "FROM training_logs l JOIN training_sets s ON l.id = s.log_id "
      "GROUP BY exercise_name "
      "ORDER BY exercise_name ASC;";

  std::vector<PRRecord> records;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      PRRecord record;
      record.exercise_name_ =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      record.max_weight_ = sqlite3_column_double(stmt, 1);
      record.reps_ = sqlite3_column_int(stmt, 2);
      record.date_ =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

      // Basic 1RM calculation logic (epley & brzycki)
      if (record.reps_ > 1) {
        // Epley formula: weight * (1 + reps/30)
        constexpr double kEpleyDivisor = 30.0;
        // Brzycki formula: weight * 36 / (37 - reps)
        constexpr double kBrzyckiMultiplier = 36.0;
        constexpr double kBrzyckiConstant = 37.0;

        record.estimated_1rm_epley_ =
            record.max_weight_ *
            (1.0 + static_cast<double>(record.reps_) / kEpleyDivisor);
        record.estimated_1rm_brzycki_ =
            record.max_weight_ * kBrzyckiMultiplier /
            (kBrzyckiConstant - static_cast<double>(record.reps_));
      } else {
        record.estimated_1rm_epley_ = record.max_weight_;
        record.estimated_1rm_brzycki_ = record.max_weight_;
      }
      records.push_back(record);
    }
    sqlite3_finalize(stmt);
  }
  return records;
}
