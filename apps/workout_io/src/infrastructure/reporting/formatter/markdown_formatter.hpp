// report/formatter/markdown_formatter.hpp

#ifndef REPORT_FORMATTER_MARKDOWN_FORMATTER_HPP_
#define REPORT_FORMATTER_MARKDOWN_FORMATTER_HPP_

#include "../database/database_manager.hpp"
#include <filesystem>
#include <iostream>
#include <map>
#include <ostream>
#include <string_view>
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
                               const std::string& output_dir,
                               const std::string& display_unit) -> bool;

  static auto BuildTypeReportMarkdown(const std::string& cycle_id,
                                      const std::string& type,
                                      const CycleData& cycle_data,
                                      const std::string& display_unit)
      -> std::string;

private:
  struct TypeSummary {
    double total_volume_kg_ = 0.0;
    std::string common_original_unit_;
    double average_intensity_kg_ = 0.0;
    int session_count_ = 0;
    int total_reps_ = 0;
    int total_sets_ = 0;
    double vol_power_kg_ = 0.0;
    double vol_hypertrophy_kg_ = 0.0;
    double vol_endurance_kg_ = 0.0;
  };

  struct SetGroup {
    double weight_kg_;
    std::string original_unit_;
    double original_weight_value_;
    std::string note_;
    std::vector<int> reps_list_;
    double volume_kg_;
    double estimated_1rm_kg_;
  };

  static auto BuildTypeSummary(const std::vector<LogEntry>& type_logs)
      -> TypeSummary;
  static auto GroupSets(const std::vector<SetDetail>& sets,
                        std::string_view display_unit) -> std::vector<SetGroup>;
  static auto FormatExercise(std::ostream& md_file, const LogEntry& log,
                             std::string_view display_unit) -> void;

  static auto ProcessCycle(const std::string& cycle_id,
                          const CycleData& cycle_data,
                          const std::string& output_dir,
                          const std::string& display_unit) -> void;

  struct ReportParams {
    std::string_view cycle_id;
    std::string_view type;
    std::string_view display_unit;
  };

  static auto ProcessType(const ReportParams& params,
                         const std::vector<LogEntry>& type_logs,
                         const CycleData& cycle_summary,
                         const std::filesystem::path& cycle_dir) -> void;

  static auto ExportSummary(const std::vector<PRRecord>& prs,
                            const std::string& output_dir,
                            const std::string& display_unit) -> void;

  static auto WriteTypeReportMarkdown(std::ostream& output,
                                      const ReportParams& params,
                                      const std::vector<LogEntry>& type_logs,
                                      const CycleData& cycle_summary) -> void;

  static auto ProcessDateGroup(std::ostream& md_file, const std::string& date,
                               const std::vector<LogEntry>& daily_entries,
                               std::string_view display_unit)
      -> void;
};

#endif // REPORT_FORMATTER_MARKDOWN_FORMATTER_HPP_
