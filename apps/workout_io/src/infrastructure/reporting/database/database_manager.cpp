// report/database/database_manager.cpp

#include "infrastructure/reporting/database/database_manager.hpp"

#include <iostream>
#include <map>
#include <optional>

#include "domain/services/training_metrics_service.hpp"

auto DatabaseManager::QueryAllLogs(sqlite3* sqlite_db)
    -> std::map<std::string, CycleData> {
  std::cout << "Querying data from database..." << std::endl;
  std::map<std::string, CycleData> data_by_cycle;
  sqlite3_stmt* stmt = nullptr;

  const char* sql =
      "SELECT l.cycle_id, l.total_days, l.exercise_type, l.id, "
      "l.date, l.exercise_name, l.daily_note, l.project_note, "
      "s.reps, s.weight_kg, s.original_unit, s.original_weight_value, "
      "s.volume, s.set_note "
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
    constexpr int kColWeightKg = 9;
    constexpr int kColOriginalUnit = 10;
    constexpr int kColOriginalWeightValue = 11;
    constexpr int kColVolume = 12;
    constexpr int kColSetNote = 13;

    std::string cycle_id =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColCycleId));
    int total_days = sqlite3_column_int(stmt, kColTotalDays);
    std::string type =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColType));
    long long log_id = sqlite3_column_int64(stmt, kColLogId);

    if (!data_by_cycle.contains(cycle_id)) {
      data_by_cycle[cycle_id].total_days_ = total_days;
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
    detail.weight_kg_ = sqlite3_column_double(stmt, kColWeightKg);

    const unsigned char* unit_text =
        sqlite3_column_text(stmt, kColOriginalUnit);
    detail.original_unit_ = (unit_text != nullptr)
                                ? reinterpret_cast<const char*>(unit_text)
                                : "kg";
    detail.original_weight_value_ =
        sqlite3_column_double(stmt, kColOriginalWeightValue);
    detail.volume_ = sqlite3_column_double(stmt, kColVolume);
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

  std::cout << "Data queried successfully." << std::endl;
  return data_by_cycle;
}

auto DatabaseManager::QueryCycleTypeLogs(sqlite3* sqlite_db,
                                         const std::string& cycle_id,
                                         const std::string& exercise_type)
    -> std::optional<CycleData> {
  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT l.total_days, l.id, l.date, l.exercise_name, l.daily_note, "
      "l.project_note, s.reps, s.weight_kg, s.original_unit, "
      "s.original_weight_value, s.volume, s.set_note "
      "FROM training_logs l "
      "JOIN training_sets s ON l.id = s.log_id "
      "WHERE l.cycle_id = ?1 AND l.exercise_type = ?2 "
      "ORDER BY l.date, l.id, s.set_number;";

  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Failed to prepare cycle type query statement: "
              << sqlite3_errmsg(sqlite_db) << std::endl;
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, cycle_id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, exercise_type.c_str(), -1, SQLITE_TRANSIENT);

  CycleData cycle_data{.total_days_ = 0, .logs_ = {}};
  std::map<long long, LogEntry> temp_entries;
  bool has_rows = false;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    has_rows = true;

    constexpr int kColTotalDays = 0;
    constexpr int kColLogId = 1;
    constexpr int kColDate = 2;
    constexpr int kColExerciseName = 3;
    constexpr int kColDailyNote = 4;
    constexpr int kColProjectNote = 5;
    constexpr int kColReps = 6;
    constexpr int kColWeightKg = 7;
    constexpr int kColOriginalUnit = 8;
    constexpr int kColOriginalWeightValue = 9;
    constexpr int kColVolume = 10;
    constexpr int kColSetNote = 11;

    cycle_data.total_days_ = sqlite3_column_int(stmt, kColTotalDays);
    const long long log_id = sqlite3_column_int64(stmt, kColLogId);

    if (!temp_entries.contains(log_id)) {
      LogEntry entry;
      entry.date_ =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColDate));
      entry.exercise_name_ = reinterpret_cast<const char*>(
          sqlite3_column_text(stmt, kColExerciseName));
      const unsigned char* note_text = sqlite3_column_text(stmt, kColDailyNote);
      entry.daily_note_ =
          note_text != nullptr ? reinterpret_cast<const char*>(note_text) : "";
      const unsigned char* project_note_text =
          sqlite3_column_text(stmt, kColProjectNote);
      entry.project_note_ =
          project_note_text != nullptr
              ? reinterpret_cast<const char*>(project_note_text)
              : "";
      entry.exercise_type_ = exercise_type;
      temp_entries[log_id] = entry;
    }

    SetDetail detail;
    detail.reps_ = sqlite3_column_int(stmt, kColReps);
    detail.weight_kg_ = sqlite3_column_double(stmt, kColWeightKg);
    const unsigned char* unit_text =
        sqlite3_column_text(stmt, kColOriginalUnit);
    detail.original_unit_ =
        unit_text != nullptr ? reinterpret_cast<const char*>(unit_text) : "kg";
    detail.original_weight_value_ =
        sqlite3_column_double(stmt, kColOriginalWeightValue);
    detail.volume_ = sqlite3_column_double(stmt, kColVolume);
    const unsigned char* set_note_text = sqlite3_column_text(stmt, kColSetNote);
    detail.note_ = set_note_text != nullptr
                       ? reinterpret_cast<const char*>(set_note_text)
                       : "";
    temp_entries[log_id].sets_.push_back(detail);
  }

  sqlite3_finalize(stmt);

  if (!has_rows) {
    return std::nullopt;
  }

  for (auto& [log_id, entry] : temp_entries) {
    (void)log_id;
    cycle_data.logs_.push_back(std::move(entry));
  }

  return cycle_data;
}

auto DatabaseManager::QueryPRSummary(sqlite3* sqlite_db)
    -> std::vector<PRRecord> {
  const char* sql =
      "WITH ranked_prs AS ("
      "  SELECT l.exercise_name, s.weight_kg, s.original_unit, "
      "         s.original_weight_value, s.reps, l.date, "
      "         ROW_NUMBER() OVER ("
      "           PARTITION BY l.exercise_name "
      "           ORDER BY s.weight_kg DESC, s.reps DESC, l.date DESC, "
      "                    s.id DESC"
      "         ) AS rank_index "
      "  FROM training_logs l "
      "  JOIN training_sets s ON l.id = s.log_id"
      ") "
      "SELECT exercise_name, weight_kg, original_unit, original_weight_value, "
      "       reps, date "
      "FROM ranked_prs "
      "WHERE rank_index = 1 "
      "ORDER BY exercise_name ASC;";

  std::vector<PRRecord> records;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      PRRecord record;
      record.exercise_name_ =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      record.max_weight_kg_ = sqlite3_column_double(stmt, 1);
      const unsigned char* unit_text = sqlite3_column_text(stmt, 2);
      record.original_unit_ = (unit_text != nullptr)
                                  ? reinterpret_cast<const char*>(unit_text)
                                  : "kg";
      record.original_weight_value_ = sqlite3_column_double(stmt, 3);
      record.reps_ = sqlite3_column_int(stmt, 4);
      record.date_ =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

      record.estimated_1rm_epley_ = TrainingMetricsService::EstimateOneRmEpley(
          record.max_weight_kg_, record.reps_);
      record.estimated_1rm_brzycki_ =
          TrainingMetricsService::EstimateOneRmBrzycki(record.max_weight_kg_,
                                                       record.reps_);
      records.push_back(record);
    }
    sqlite3_finalize(stmt);
  }
  return records;
}
