// src/formatter/MarkdownFormatter.hpp

#ifndef MARKDOWN_FORMATTER_H
#define MARKDOWN_FORMATTER_H

#include "../database/DatabaseManager.hpp" 
#include <string>
#include <vector>
#include <map>

class MarkdownFormatter {
public:
    /**
     * @brief 将查询到的数据按训练周期导出为Markdown文件。
     * @param data_by_cycle 按 cycle_id 分组的日志数据。
     * @param output_dir 要保存 .md 文件的目标目录路径。
     * @return 如果成功导出所有文件，则返回 true。
     */
    // [MODIFIED] 更改了函数参数
    static bool export_to_markdown(const std::map<std::string, CycleData>& data_by_cycle, const std::string& output_dir);
};

#endif // MARKDOWN_FORMATTER_H