// report/formatter/markdown_formatter.hpp

#ifndef REPORT_FORMATTER_MARKDOWN_FORMATTER_HPP_
#define REPORT_FORMATTER_MARKDOWN_FORMATTER_HPP_

#include "../database/database_manager.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

class MarkdownFormatter {
public:
  /**
   * @brief 将查询到的数据按训练周期导出为 Markdown 文件。
   * @param data_by_cycle 按 cycle_id 分组的日志数据。
   * @param output_dir 要保存 .md 文件的目标目录路径。
   * @return 如果成功导出所有文件，则返回 true。
   */
  static auto ExportToMarkdown(const std::map<std::string, CycleData>& data_by_cycle,
                               const std::vector<PRRecord>& prs,
                               const std::string& output_dir) -> bool;

private:
  struct SetGroup {
    double weight_;
    std::string unit_;
    double elastic_band_;
    std::string note_;
    std::vector<int> reps_list_;
    double volume_;
    double estimated_1rm_;
  };

  static auto GroupSets(const std::vector<SetDetail>& sets) -> std::vector<SetGroup>;
  static auto FormatExercise(std::ostream& md_file, const LogEntry& log) -> void;

  static auto ProcessCycle(const std::string& cycle_id,
                          const CycleData& cycle_data,
                          const std::string& output_dir) -> void;

  struct ReportParams {
    std::string_view cycle_id;
    std::string_view type;
  };

  static auto ProcessType(const ReportParams& params,
                         const std::vector<LogEntry>& type_logs,
                         const CycleData& cycle_summary,
                         const std::filesystem::path& cycle_dir) -> void;

  static auto ExportSummary(const std::vector<PRRecord>& prs,
                            const std::string& output_dir) -> void;

  static auto ProcessDateGroup(std::ofstream& md_file, const std::string& date,
                              const std::vector<LogEntry>& daily_entries)
      -> void;
};

#endif // REPORT_FORMATTER_MARKDOWN_FORMATTER_HPP_
