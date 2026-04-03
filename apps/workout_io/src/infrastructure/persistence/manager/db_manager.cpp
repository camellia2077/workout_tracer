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
      "  weight_kg REAL NOT NULL,"
      "  original_unit TEXT NOT NULL DEFAULT 'kg',"
      "  original_weight_value REAL NOT NULL,"
      "  reps INTEGER NOT NULL,"
      "  volume REAL NOT NULL,"
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
  return true;
}
