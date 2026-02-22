// db/facade/db_facade.cpp

#include "infrastructure/persistence/facade/db_facade.hpp"

#include <iostream>
#include <stdexcept>

#include "infrastructure/persistence/inserter/data_inserter.hpp"

auto DbFacade::InsertTrainingData(sqlite3* db_connection,
                                  const std::vector<DailyData>& data) -> bool {
  char* z_err_msg = nullptr;

  if (sqlite3_exec(db_connection, "BEGIN TRANSACTION;", nullptr, nullptr,
                   &z_err_msg) != SQLITE_OK) {
    std::cerr << "SQL error starting transaction: " << z_err_msg << std::endl;
    sqlite3_free(z_err_msg);
    return false;
  }

  try {
    DataInserter inserter(db_connection);
    if (!inserter.Insert(data)) {
      sqlite3_exec(db_connection, "ROLLBACK;", nullptr, nullptr, nullptr);
      return false;
    }
  } catch (const std::exception& e) {
    std::cerr << "An error occurred during insertion: " << e.what()
              << std::endl;
    sqlite3_exec(db_connection, "ROLLBACK;", nullptr, nullptr, nullptr);
    return false;
  }

  if (sqlite3_exec(db_connection, "COMMIT;", nullptr, nullptr, &z_err_msg) !=
      SQLITE_OK) {
    std::cerr << "SQL error committing transaction: " << z_err_msg << std::endl;
    sqlite3_free(z_err_msg);
    sqlite3_exec(db_connection, "ROLLBACK;", nullptr, nullptr, nullptr);
    return false;
  }

  return true;
}