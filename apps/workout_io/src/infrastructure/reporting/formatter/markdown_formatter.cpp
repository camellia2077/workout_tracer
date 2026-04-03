// report/formatter/markdown_formatter.cpp

#include "infrastructure/reporting/formatter/markdown_formatter.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "domain/services/training_metrics_service.hpp"
#include "domain/services/weight_unit_service.hpp"

namespace fs = std::filesystem;

auto MarkdownFormatter::BuildTypeSummary(const std::vector<LogEntry>& type_logs)
    -> TypeSummary {
  TypeSummary summary;
  summary.session_count_ = static_cast<int>(type_logs.size());

  bool has_unit = false;
  std::string common_unit;

  for (const auto& log : type_logs) {
    for (const auto& set_item : log.sets_) {
      summary.total_sets_++;
      summary.total_reps_ += set_item.reps_;
      summary.total_volume_kg_ += set_item.volume_;

      if (set_item.reps_ >= 1 && set_item.reps_ <= 5) {
        summary.vol_power_kg_ += set_item.volume_;
      } else if (set_item.reps_ >= 6 && set_item.reps_ <= 12) {
        summary.vol_hypertrophy_kg_ += set_item.volume_;
      } else if (set_item.reps_ >= 13) {
        summary.vol_endurance_kg_ += set_item.volume_;
      }

      if (!has_unit) {
        common_unit = set_item.original_unit_;
        has_unit = true;
      } else if (common_unit != set_item.original_unit_) {
        common_unit.clear();
      }
    }
  }

  if (summary.total_reps_ > 0) {
    summary.average_intensity_kg_ =
        summary.total_volume_kg_ / static_cast<double>(summary.total_reps_);
  }

  if (!common_unit.empty()) {
    summary.common_original_unit_ = common_unit;
  }

  return summary;
}

auto MarkdownFormatter::GroupSets(const std::vector<SetDetail>& sets,
                                  std::string_view display_unit)
    -> std::vector<SetGroup> {
  std::vector<SetGroup> groups;

  constexpr double kEpsilon = 0.001;
  const std::string normalized_display =
      WeightUnitService::NormalizeDisplayUnit(display_unit)
          .value_or(std::string(WeightUnitService::kDisplayOriginal));
  auto is_same_load = [&](const SetDetail& set_detail,
                          const SetGroup& set_group) -> bool {
    if (normalized_display == WeightUnitService::kDisplayOriginal) {
      return std::abs(set_detail.original_weight_value_ -
                      set_group.original_weight_value_) < kEpsilon &&
             set_detail.original_unit_ == set_group.original_unit_ &&
             set_detail.note_ == set_group.note_;
    }

    return std::abs(set_detail.weight_kg_ - set_group.weight_kg_) < kEpsilon &&
           set_detail.note_ == set_group.note_;
  };

  for (const auto& set_item : sets) {
    if (!groups.empty() && is_same_load(set_item, groups.back())) {
      groups.back().reps_list_.push_back(set_item.reps_);
      groups.back().volume_kg_ += set_item.volume_;
    } else {
      SetGroup group;
      group.weight_kg_ = set_item.weight_kg_;
      group.original_unit_ = set_item.original_unit_;
      group.original_weight_value_ = set_item.original_weight_value_;
      group.note_ = set_item.note_;
      group.reps_list_ = {set_item.reps_};
      group.volume_kg_ = set_item.volume_;
      group.estimated_1rm_kg_ = 0.0;
      groups.push_back(group);
    }
  }

  for (auto& group : groups) {
    int max_reps = 0;
    for (int reps : group.reps_list_) {
      max_reps = std::max(max_reps, reps);
    }
    if (group.weight_kg_ > 0.0) {
      group.estimated_1rm_kg_ = TrainingMetricsService::EstimateOneRmEpley(
          group.weight_kg_, max_reps);
    }
  }

  return groups;
}

auto MarkdownFormatter::FormatExercise(std::ostream& md_file,
                                       const LogEntry& log,
                                       std::string_view display_unit) -> void {
  md_file << "  - **" << log.exercise_name_ << "**\n";
  if (!log.project_note_.empty()) {
    md_file << "    - **Note:** " << log.project_note_ << "\n";
  }

  if (log.sets_.empty()) {
    return;
  }

  auto groups = GroupSets(log.sets_, display_unit);

  for (const auto& group : groups) {
    std::stringstream report_stream;
    const auto display_weight = WeightUnitService::ResolveDetailWeight(
        group.weight_kg_, group.original_unit_, group.original_weight_value_,
        display_unit);
    const std::string metric_unit = WeightUnitService::ResolveDetailDisplayUnit(
        display_unit, group.original_unit_);
    const double volume_display =
        WeightUnitService::ConvertFromKg(group.volume_kg_, metric_unit);
    const double e1rm_display =
        WeightUnitService::ConvertFromKg(group.estimated_1rm_kg_, metric_unit);

    report_stream << WeightUnitService::FormatWeightWithUnit(
        display_weight.value_, display_weight.unit_);

    report_stream << " x [";
    for (size_t i = 0; i < group.reps_list_.size(); ++i) {
      report_stream << group.reps_list_[i];
      if (i < group.reps_list_.size() - 1) {
        report_stream << ", ";
      }
    }
    report_stream << "]";

    md_file << "    - `" << report_stream.str() << "`";
    md_file << " (Vol: " << WeightUnitService::FormatMetricValue(volume_display)
            << metric_unit;
    if (group.weight_kg_ > 0.0 &&
        (group.reps_list_[0] > 1 || group.reps_list_.size() > 1)) {
      md_file << ", e1RM: "
              << WeightUnitService::FormatMetricValue(e1rm_display)
              << metric_unit;
    }
    md_file << ")\n";

    if (!group.note_.empty()) {
      md_file << "      - **Note:** " << group.note_ << "\n";
    }
  }
}

auto MarkdownFormatter::ExportToMarkdown(
    const std::map<std::string, CycleData>& data_by_cycle,
    const std::vector<PRRecord>& prs, const std::string& output_dir,
    const std::string& display_unit) -> bool {
  std::cout << "Exporting data to Markdown files (Enhanced Reporting System)..."
            << std::endl;

  try {
    fs::create_directories(output_dir);
  } catch (const fs::filesystem_error& e) {
    std::cerr << "Error creating root directory " << output_dir << ": "
              << e.what() << std::endl;
    return false;
  }

  ExportSummary(prs, output_dir, display_unit);

  for (const auto& [cycle_id, cycle_data] : data_by_cycle) {
    ProcessCycle(cycle_id, cycle_data, output_dir, display_unit);
  }

  std::cout << "Markdown export completed." << std::endl;
  return true;
}

auto MarkdownFormatter::BuildTypeReportMarkdown(const std::string& cycle_id,
                                                const std::string& type,
                                                const CycleData& cycle_data,
                                                const std::string& display_unit)
    -> std::string {
  std::ostringstream output;
  WriteTypeReportMarkdown(
      output,
      {.cycle_id = cycle_id, .type = type, .display_unit = display_unit},
      cycle_data.logs_, cycle_data);
  return output.str();
}

auto MarkdownFormatter::ProcessCycle(const std::string& cycle_id,
                                     const CycleData& cycle_data,
                                     const std::string& output_dir,
                                     const std::string& display_unit) -> void {
  fs::path cycle_dir = fs::path(output_dir) / cycle_id;
  try {
    fs::create_directories(cycle_dir);
  } catch (const fs::filesystem_error& e) {
    std::cerr << "Error creating cycle directory " << cycle_dir << ": "
              << e.what() << std::endl;
    return;
  }

  std::cout << "  -> Processing Cycle: " << cycle_id
            << " into folder: " << cycle_dir << std::endl;

  std::map<std::string, std::vector<LogEntry>> logs_by_type;
  for (const auto& log : cycle_data.logs_) {
    std::string type_key =
        log.exercise_type_.empty() ? "unknown" : log.exercise_type_;
    logs_by_type[type_key].push_back(log);
  }

  for (const auto& [type, type_logs] : logs_by_type) {
    ProcessType(
        {.cycle_id = cycle_id, .type = type, .display_unit = display_unit},
        type_logs, cycle_data, cycle_dir);
  }
}

auto MarkdownFormatter::ProcessType(const ReportParams& params,
                                    const std::vector<LogEntry>& type_logs,
                                    const CycleData& cycle_data,
                                    const std::filesystem::path& cycle_dir)
    -> void {
  fs::path file_path = cycle_dir / (std::string(params.type) + ".md");
  std::ofstream md_file(file_path);

  if (!md_file.is_open()) {
    std::cerr << "Error: Could not open file: " << file_path << std::endl;
    return;
  }

  WriteTypeReportMarkdown(md_file, params, type_logs, cycle_data);
}

auto MarkdownFormatter::WriteTypeReportMarkdown(
    std::ostream& md_file, const ReportParams& params,
    const std::vector<LogEntry>& type_logs, const CycleData& cycle_data)
    -> void {
  std::string display_title(params.type);
  if (!display_title.empty()) {
    display_title[0] = static_cast<char>(toupper(display_title[0]));
  }

  md_file << "# " << display_title << " Training Report\n\n";
  md_file << "**Cycle:** `" << params.cycle_id << "`\n\n";

  const TypeSummary summary = BuildTypeSummary(type_logs);
  const std::string aggregate_unit =
      WeightUnitService::ResolveAggregateDisplayUnit(
          params.display_unit, summary.common_original_unit_);
  const double total_volume = WeightUnitService::ConvertFromKg(
      summary.total_volume_kg_, aggregate_unit);
  const double average_intensity = WeightUnitService::ConvertFromKg(
      summary.average_intensity_kg_, aggregate_unit);
  md_file << "## Kinematics Dashboard\n";
  md_file << "| Metric | Value |\n";
  md_file << "| :--- | :--- |\n";
  md_file << "| **Total Volume** | "
          << WeightUnitService::FormatMetricValue(total_volume) << " "
          << aggregate_unit << " |\n";
  md_file << "| **Avg Intensity** | "
          << WeightUnitService::FormatMetricValue(average_intensity) << " "
          << aggregate_unit << "/rep |\n";
  md_file << "| **Frequency** | "
          << TrainingMetricsService::SessionsPerWeek(summary.session_count_,
                                                     cycle_data.total_days_)
          << " sessions/week |\n";

  auto get_percent = [&](double vol) -> double {
    return TrainingMetricsService::PercentageOfTotal(vol,
                                                     summary.total_volume_kg_);
  };
  md_file << "| **Dist. Power (1-5)** | " << get_percent(summary.vol_power_kg_)
          << "% |\n";
  md_file << "| **Dist. Hyper (6-12)** | "
          << get_percent(summary.vol_hypertrophy_kg_) << "% |\n";
  md_file << "| **Dist. Endure (13+)** | "
          << get_percent(summary.vol_endurance_kg_) << "% |\n\n";

  md_file << "---\n\n";

  std::map<std::string, std::vector<LogEntry>> daily_logs_for_type;
  for (const auto& log : type_logs) {
    daily_logs_for_type[log.date_].push_back(log);
  }

  for (const auto& [date, daily_entries] : daily_logs_for_type) {
    ProcessDateGroup(md_file, date, daily_entries, params.display_unit);
  }
}

auto MarkdownFormatter::ProcessDateGroup(
    std::ostream& md_file, const std::string& date,
    const std::vector<LogEntry>& daily_entries, std::string_view display_unit)
    -> void {
  md_file << "## " << date << "\n\n";

  std::string daily_note;
  for (const auto& log : daily_entries) {
    if (!log.daily_note_.empty()) {
      daily_note = log.daily_note_;
      break;
    }
  }
  if (!daily_note.empty()) {
    md_file << "**Note:** " << daily_note << "\n\n";
  }

  for (const auto& log : daily_entries) {
    FormatExercise(md_file, log, display_unit);
  }
  md_file << "\n---\n\n";
}

auto MarkdownFormatter::ExportSummary(const std::vector<PRRecord>& prs,
                                      const std::string& output_dir,
                                      const std::string& display_unit) -> void {
  fs::path summary_path = fs::path(output_dir) / "Summary.md";
  std::ofstream md_file(summary_path);

  if (!md_file.is_open()) {
    std::cerr << "Error: Could not open summary file: " << summary_path
              << std::endl;
    return;
  }

  md_file << "# Training Hall of Fame\n\n";
  md_file << "Generated on: " << __DATE__ << "\n\n";

  md_file << "## Personal Records (PRs)\n";
  md_file << "| Exercise | Max Weight | Reps | Date | Est. 1RM (Epley) | Est. "
             "1RM (Brzycki) |\n";
  md_file << "| :--- | :--- | :--- | :--- | :--- | :--- |\n";

  for (const auto& pr : prs) {
    const auto display_weight = WeightUnitService::ResolveDetailWeight(
        pr.max_weight_kg_, pr.original_unit_, pr.original_weight_value_,
        display_unit);
    const std::string metric_unit = WeightUnitService::ResolveDetailDisplayUnit(
        display_unit, pr.original_unit_);
    const double epley_display =
        WeightUnitService::ConvertFromKg(pr.estimated_1rm_epley_, metric_unit);
    const double brzycki_display = WeightUnitService::ConvertFromKg(
        pr.estimated_1rm_brzycki_, metric_unit);

    md_file << "| **" << pr.exercise_name_ << "** | "
            << WeightUnitService::FormatWeightValue(display_weight.value_)
            << " " << display_weight.unit_ << " | " << pr.reps_ << " | "
            << pr.date_ << " | ";
    if (pr.max_weight_kg_ > 0.0 && pr.reps_ > 1) {
      md_file << WeightUnitService::FormatMetricValue(epley_display) << " "
              << metric_unit << " | "
              << WeightUnitService::FormatMetricValue(brzycki_display) << " "
              << metric_unit << " |\n";
    } else {
      md_file << "- | - |\n";
    }
  }

  md_file << "\n---\n*Keep pushing your limits!*";
}
