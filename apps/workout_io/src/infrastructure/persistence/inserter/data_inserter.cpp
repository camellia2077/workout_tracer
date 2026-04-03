// db/inserter/data_inserter.cpp

#include "infrastructure/persistence/inserter/data_inserter.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

DataInserter::DataInserter(sqlite3* db_handle) : db_(db_handle) {}

namespace {
auto NormalizeCycleId(const std::string& date) -> std::string {
  // Canonical cycle granularity is monthly: YYYY-MM.
  if (date.size() >= 7 && date[4] == '-') {
    return date.substr(0, 7);
  }
  return date;
}
}  // namespace

auto DataInserter::InsertSets(sqlite3_stmt* stmt_set, sqlite3_int64 log_id,
                              const std::vector<SetData>& sets) -> void {
  for (const auto& set_item : sets) {
    sqlite3_bind_int64(stmt_set, kColSetLogId, log_id);
    sqlite3_bind_int(stmt_set, kColSetNumber, set_item.set_number_);
    sqlite3_bind_double(stmt_set, kColSetWeightKg, set_item.weight_kg_);
    sqlite3_bind_text(stmt_set, kColSetOriginalUnit,
                      set_item.original_unit_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt_set, kColSetOriginalWeightValue,
                        set_item.original_weight_value_);
    sqlite3_bind_int(stmt_set, kColSetReps, set_item.reps_);
    sqlite3_bind_double(stmt_set, kColSetVolume, set_item.volume_);
    sqlite3_bind_text(stmt_set, kColSetNote, set_item.note_.c_str(), -1,
                      SQLITE_STATIC);

    if (sqlite3_step(stmt_set) != SQLITE_DONE) {
      throw std::runtime_error("Error inserting training set: " +
                               std::string(sqlite3_errmsg(db_)));
    }
    sqlite3_reset(stmt_set);
  }
}

auto DataInserter::Insert(const std::vector<DailyData>& data) -> bool {
  if (data.empty()) {
    return false;
  }

  const char* sql_insert_log =
      "INSERT INTO training_logs (cycle_id, total_days, date, daily_note, "
      "project_note, exercise_name, exercise_type, total_volume) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
  const char* sql_insert_set =
      "INSERT INTO training_sets (log_id, set_number, weight_kg, "
      "original_unit, original_weight_value, reps, volume, set_note) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt_log = nullptr;
  sqlite3_stmt* stmt_set = nullptr;

  if (sqlite3_prepare_v2(db_, sql_insert_log, -1, &stmt_log, nullptr) !=
          SQLITE_OK ||
      sqlite3_prepare_v2(db_, sql_insert_set, -1, &stmt_set, nullptr) !=
          SQLITE_OK) {
    std::string err_msg = "Failed to prepare statement: ";
    err_msg += sqlite3_errmsg(db_);
    if (stmt_log != nullptr) {
      sqlite3_finalize(stmt_log);
    }
    if (stmt_set != nullptr) {
      sqlite3_finalize(stmt_set);
    }
    throw std::runtime_error(err_msg);
  }

  try {
    std::string cycle_id = NormalizeCycleId(data[0].date_);
    int total_days = static_cast<int>(data.size());

    for (const auto& daily : data) {
      std::string date = daily.date_;

      for (const auto& proj : daily.projects_) {
        sqlite3_bind_text(stmt_log, kColLogCycleId, cycle_id.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_log, kColLogTotalDays, total_days);
        sqlite3_bind_text(stmt_log, kColLogDate, date.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_log, kColLogDailyNote, daily.note_.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_log, kColLogProjectNote, proj.note_.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt_log, kColLogExerciseName,
                          proj.project_name_.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_log, kColLogExerciseType, proj.type_.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_double(stmt_log, kColLogTotalVolume, proj.total_volume_);

        if (sqlite3_step(stmt_log) != SQLITE_DONE) {
          throw std::runtime_error("Error inserting training log: " +
                                   std::string(sqlite3_errmsg(db_)));
        }
        sqlite3_reset(stmt_log);

        sqlite3_int64 last_log_id = sqlite3_last_insert_rowid(db_);
        InsertSets(stmt_set, last_log_id, proj.sets_);
      }
    }
  } catch (const std::exception& e) {
    sqlite3_finalize(stmt_log);
    sqlite3_finalize(stmt_set);
    throw;
  }

  sqlite3_finalize(stmt_log);
  sqlite3_finalize(stmt_set);
  return true;
}
