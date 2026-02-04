// db/manager/db_manager.hpp

#ifndef DB_MANAGER_DB_MANAGER_HPP_
#define DB_MANAGER_DB_MANAGER_HPP_

#include "sqlite3.h"
#include <string>

class DbManager {
public:
  explicit DbManager(std::string db_path);
  ~DbManager();

  // Disable copy and assignment
  DbManager(const DbManager&) = delete;
  auto operator=(const DbManager&) -> DbManager& = delete;

  auto Open() -> bool;
  auto Close() -> void;
  auto GetConnection() const -> sqlite3*;

private:
  std::string db_path_;
  sqlite3* db_ = nullptr;
  
  auto CreateTables() -> bool;
};

#endif // DB_MANAGER_DB_MANAGER_HPP_