// src/report/formatter/MarkdownFormatter.hpp

#ifndef MARKDOWN_FORMATTER_H
#define MARKDOWN_FORMATTER_H

#include "../database/DatabaseManager.hpp" 
#include <string>
#include <vector>
#include <map>
#include <iostream> // 需要 ostream

class MarkdownFormatter {
public:
    /**
     * @brief 将查询到的数据按训练周期导出为Markdown文件。
     * @param data_by_cycle 按 cycle_id 分组的日志数据。
     * @param output_dir 要保存 .md 文件的目标目录路径。
     * @return 如果成功导出所有文件，则返回 true。
     */
    static bool export_to_markdown(const std::map<std::string, CycleData>& data_by_cycle, const std::string& output_dir);

private:
    // [NEW] 辅助结构体：用于在打印前聚合相同配置的组
    struct SetGroup {
        double weight;
        std::string unit;
        double elastic_band;
        std::vector<int> reps_list;
    };

    // [NEW] 辅助函数声明
    static std::vector<SetGroup> groupSets(const std::vector<SetDetail>& sets);
    static void formatExercise(std::ostream& md_file, const LogEntry& log);
};

#endif // MARKDOWN_FORMATTER_H