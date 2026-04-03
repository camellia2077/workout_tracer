#ifndef APPLICATION_INTERFACES_I_REPORT_EXPORTER_HPP_
#define APPLICATION_INTERFACES_I_REPORT_EXPORTER_HPP_

#include <string>

#include "core/application/use_case_result.hpp"

class IReportExporter {
public:
  virtual ~IReportExporter() = default;

  [[nodiscard]] virtual auto ExportReports(const std::string& output_dir,
                                           const std::string& display_unit,
                                           const std::string& cycle_id_filter)
      -> UseCaseResult<void> = 0;
};

#endif  // APPLICATION_INTERFACES_I_REPORT_EXPORTER_HPP_
