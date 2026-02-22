#ifndef APPLICATION_INTERFACES_I_REPORT_EXPORTER_HPP_
#define APPLICATION_INTERFACES_I_REPORT_EXPORTER_HPP_

#include <string>

class IReportExporter {
public:
  virtual ~IReportExporter() = default;

  [[nodiscard]] virtual auto ExportReports(const std::string& output_dir)
      -> bool = 0;
};

#endif  // APPLICATION_INTERFACES_I_REPORT_EXPORTER_HPP_
