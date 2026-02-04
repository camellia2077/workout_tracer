// report/facade/report_facade.cpp

#include "infrastructure/reporting/facade/report_facade.hpp"

#include <iostream>

#include "infrastructure/reporting/database/database_manager.hpp"
#include "infrastructure/reporting/formatter/markdown_formatter.hpp"

auto ReportFacade::GenerateReport(sqlite3* sqlite_db, const std::string& output_dir)
    -> bool {
  std::cout << "Starting report generation..." << std::endl;

  auto data = DatabaseManager::QueryAllLogs(sqlite_db);
  auto prs = DatabaseManager::QueryPRSummary(sqlite_db);

  if (data.empty()) {
    std::cout << "No data found in the database to export." << std::endl;
    return true;
  }

  return MarkdownFormatter::ExportToMarkdown(data, prs, output_dir);
}