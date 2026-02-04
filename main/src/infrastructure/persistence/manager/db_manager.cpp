// db/manager/db_manager.cpp

#include "infrastructure/persistence/manager/db_manager.hpp"

#include <iostream>
#include <utility>

DbManager::DbManager(std::string db_path) : db_path_(std::move(db_path)) {}

DbManager::~DbManager() {
  Close();
}

auto DbManager::Open() -> bool {
  if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
    std::cerr << "Error opening database: " << sqlite3_errmsg(db_) << std::endl;
    return false;
  }
  std::cout << "Database opened successfully at " << db_path_ << std::endl;
  return CreateTables();
}

auto DbManager::Close() -> void {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
    std::cout << "Database connection closed." << std::endl;
  }
}

auto DbManager::GetConnection() const -> sqlite3* {
  return db_;
}

auto DbManager::CreateTables() -> bool {
  const char* sql =
      "CREATE TABLE IF NOT EXISTS training_logs ("
      "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  cycle_id TEXT NOT NULL,"
      "  total_days INTEGER NOT NULL,"
      "  date TEXT NOT NULL,"
      "  daily_note TEXT DEFAULT '',"
      "  project_note TEXT DEFAULT '',"
      "  exercise_name TEXT NOT NULL,"
      "  exercise_type TEXT NOT NULL,"
      "  total_volume REAL NOT NULL"
      ");"
      "CREATE TABLE IF NOT EXISTS training_sets ("
      "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  log_id INTEGER NOT NULL,"
      "  set_number INTEGER NOT NULL,"
      "  weight REAL NOT NULL,"
      "  reps INTEGER NOT NULL,"
      "  volume REAL NOT NULL,"
      "  unit TEXT DEFAULT 'kg',"
      "  elastic_band_weight REAL DEFAULT 0.0,"
      "  set_note TEXT DEFAULT '',"
      "  FOREIGN KEY (log_id) REFERENCES training_logs (id)"
      ");";

  char* z_err_msg = nullptr;
  if (sqlite3_exec(db_, sql, nullptr, nullptr, &z_err_msg) != SQLITE_OK) {
    std::cerr << "SQL error creating tables: " << z_err_msg << std::endl;
    sqlite3_free(z_err_msg);
    return false;
  }

  std::cout << "Tables verified/created successfully." << std::endl;

  struct ColumnParams {
    std::string_view table_name;
    std::string_view column_name;
    std::string_view column_definition;
  };

  auto ensure_column = [&](const ColumnParams& params) -> bool {
    sqlite3_stmt* stmt = nullptr;
    bool has_column = false;
    std::string pragma_sql =
        "PRAGMA table_info(" + std::string(params.table_name) + ");";
    if (sqlite3_prepare_v2(db_, pragma_sql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* col_text = sqlite3_column_text(stmt, 1);
        if (col_text != nullptr &&
            std::string(reinterpret_cast<const char*>(col_text)) ==
                params.column_name) {
          has_column = true;
          break;
        }
      }
    }
    if (stmt != nullptr) {
      sqlite3_finalize(stmt);
    }
    if (has_column) {
      return true;
    }
    std::string alter_sql = "ALTER TABLE " + std::string(params.table_name) +
                            " ADD COLUMN " + std::string(params.column_definition) +
                            ";";
    if (sqlite3_exec(db_, alter_sql.c_str(), nullptr, nullptr, &z_err_msg) !=
        SQLITE_OK) {
      std::cerr << "SQL error adding " << params.column_name
                << " column: " << z_err_msg << std::endl;
      sqlite3_free(z_err_msg);
      return false;
    }
    std::cout << "Added " << params.column_name << " column to "
              << params.table_name << "." << std::endl;
    return true;
  };

  return ensure_column({.table_name = "training_logs",
                        .column_name = "daily_note",
                        .column_definition = "daily_note TEXT DEFAULT ''"}) &&
         ensure_column({.table_name = "training_logs",
                        .column_name = "project_note",
                        .column_definition = "project_note TEXT DEFAULT ''"}) &&
         ensure_column({.table_name = "training_sets",
                        .column_name = "set_note",
                        .column_definition = "set_note TEXT DEFAULT ''"});
}
