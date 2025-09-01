// src/report/ReportGenerator.hpp

#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include "sqlite3.h"
#include <string>
#include <vector>
#include <map>

class ReportGenerator {
public:
    /**
     * @brief 从数据库查询所有数据，并按类型导出为多个Markdown文件。
     * @param db sqlite3数据库连接的指针。
     * @param output_dir 要保存 .md 文件的目标目录路径。
     * @return 如果成功查询并导出所有文件，则返回 true。
     */
    static bool generate_markdown_files(sqlite3* db, const std::string& output_dir);

private:
    // 用于在内部传递数据的结构体
    struct LogEntry {
        std::string date;
        std::string exercise_name;
        std::vector<int> reps;
    };

    /**
     * @brief (内部函数) 从数据库查询所有的训练日志。
     */
    static std::map<std::string, std::vector<LogEntry>> query_all_logs(sqlite3* db);

    /**
     * @brief (内部函数) 将查询到的数据导出为Markdown文件。
     */
    static bool export_to_markdown(const std::map<std::string, std::vector<LogEntry>>& data_by_type, const std::string& output_dir);
};

#endif // REPORT_GENERATOR_H