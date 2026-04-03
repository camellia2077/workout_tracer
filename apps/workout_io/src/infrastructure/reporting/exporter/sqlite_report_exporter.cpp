#include "infrastructure/reporting/exporter/sqlite_report_exporter.hpp"

#include <utility>

#include "core/application/core_error_code.hpp"
#include "infrastructure/persistence/manager/db_manager.hpp"
#include "infrastructure/reporting/facade/report_facade.hpp"

SqliteReportExporter::SqliteReportExporter(std::string db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteReportExporter::ExportReports(const std::string& output_dir,
                                         const std::string& display_unit,
                                         const std::string& cycle_id_filter)
    -> UseCaseResult<void> {
  DbManager db_manager(db_path_);
  if (!db_manager.Open()) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kDatabaseError,
                                        "failed to open database");
  }

  if (!ReportFacade::GenerateReport(db_manager.GetConnection(), output_dir,
                                    display_unit, cycle_id_filter)) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                        "failed to export reports");
  }

  return UseCaseResult<void>::Success();
}
