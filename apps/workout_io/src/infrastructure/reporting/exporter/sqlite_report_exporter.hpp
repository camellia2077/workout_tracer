#ifndef INFRASTRUCTURE_REPORTING_EXPORTER_SQLITE_REPORT_EXPORTER_HPP_
#define INFRASTRUCTURE_REPORTING_EXPORTER_SQLITE_REPORT_EXPORTER_HPP_

#include <string>

#include "application/interfaces/i_report_exporter.hpp"

class SqliteReportExporter : public IReportExporter {
public:
  explicit SqliteReportExporter(std::string db_path);

  [[nodiscard]] auto ExportReports(const std::string& output_dir)
      -> UseCaseResult<void> override;

private:
  std::string db_path_;
};

#endif  // INFRASTRUCTURE_REPORTING_EXPORTER_SQLITE_REPORT_EXPORTER_HPP_
