// report/facade/report_facade.hpp

#ifndef REPORT_FACADE_REPORT_FACADE_HPP_
#define REPORT_FACADE_REPORT_FACADE_HPP_

#include "sqlite3.h"
#include <string>

class ReportFacade {
public:
  /**
   * @brief 生成报告的统一接口。
   * @param db sqlite3数据库连接的指针。
   * @param output_dir 要保存 .md 文件的目标目录路径。
   * @return 如果报告成功生成，则返回 true。
   */
  static auto GenerateReport(sqlite3* sqlite_db, const std::string& output_dir) -> bool;
};

#endif // REPORT_FACADE_REPORT_FACADE_HPP_