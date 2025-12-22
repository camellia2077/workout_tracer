// report/facade/ReportFacade.hpp

#ifndef REPORT_FACADE_H
#define REPORT_FACADE_H

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
    static bool generate_report(sqlite3* db, const std::string& output_dir);
};

#endif // REPORT_FACADE_H