// report/facade/report_facade.cpp

#include "infrastructure/reporting/facade/report_facade.hpp"

#include <iostream>
#include <utility>

#include "infrastructure/reporting/database/database_manager.hpp"
#include "infrastructure/reporting/formatter/markdown_formatter.hpp"

auto ReportFacade::GenerateReport(sqlite3* sqlite_db,
                                  const std::string& output_dir,
                                  const std::string& display_unit,
                                  const std::string& cycle_id_filter) -> bool {
  std::cout << "Starting report generation..." << std::endl;

  auto data_all = DatabaseManager::QueryAllLogs(sqlite_db);
  auto prs = DatabaseManager::QueryPRSummary(sqlite_db);
  decltype(data_all) data_to_export;

  if (cycle_id_filter.empty()) {
    data_to_export = std::move(data_all);
  } else {
    auto found = data_all.find(cycle_id_filter);
    if (found == data_all.end()) {
      std::cout << "No data found for cycle " << cycle_id_filter
                << ", nothing to export." << std::endl;
      return true;
    }
    data_to_export.emplace(found->first, std::move(found->second));
  }

  if (data_to_export.empty()) {
    std::cout << "No data found in the database to export." << std::endl;
    return true;
  }

  return MarkdownFormatter::ExportToMarkdown(data_to_export, prs, output_dir,
                                             display_unit);
}

auto ReportFacade::GenerateTypeReportMarkdown(sqlite3* sqlite_db,
                                              const std::string& cycle_id,
                                              const std::string& exercise_type,
                                              const std::string& display_unit)
    -> std::optional<std::string> {
  auto cycle_data =
      DatabaseManager::QueryCycleTypeLogs(sqlite_db, cycle_id, exercise_type);
  if (!cycle_data.has_value()) {
    return std::nullopt;
  }

  return MarkdownFormatter::BuildTypeReportMarkdown(
      cycle_id, exercise_type, cycle_data.value(), display_unit);
}
