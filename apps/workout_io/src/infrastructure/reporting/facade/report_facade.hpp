// report/facade/report_facade.hpp

#ifndef REPORT_FACADE_REPORT_FACADE_HPP_
#define REPORT_FACADE_REPORT_FACADE_HPP_

#include "sqlite3.h"
#include <optional>
#include <string>

class ReportFacade {
public:
  /**
   * @brief 生成报告的统一接口。
   * @param db sqlite3数据库连接的指针。
   * @param output_dir 要保存 .md 文件的目标目录路径。
   * @param cycle_id_filter 可选的 cycle 过滤（YYYY-MM）。为空时导出全部。
   * @return 如果报告成功生成，则返回 true。
   */
  static auto GenerateReport(sqlite3* sqlite_db, const std::string& output_dir,
                             const std::string& display_unit,
                             const std::string& cycle_id_filter) -> bool;

  static auto GenerateTypeReportMarkdown(sqlite3* sqlite_db,
                                         const std::string& cycle_id,
                                         const std::string& exercise_type,
                                         const std::string& display_unit)
      -> std::optional<std::string>;
};

#endif // REPORT_FACADE_REPORT_FACADE_HPP_
