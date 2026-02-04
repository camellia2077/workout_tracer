// report/formatter/markdown_formatter.cpp

#include "infrastructure/reporting/formatter/markdown_formatter.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

auto MarkdownFormatter::GroupSets(const std::vector<SetDetail>& sets)
    -> std::vector<SetGroup> {
  std::vector<SetGroup> groups;

  constexpr double kEpsilon = 0.001;
  auto is_same_load =
      [](const SetDetail& set_detail, const SetGroup& set_group) -> bool {
    return std::abs(set_detail.weight_ - set_group.weight_) < kEpsilon &&
           set_detail.unit_ == set_group.unit_ &&
           std::abs(set_detail.elastic_band_weight_ - set_group.elastic_band_) <
               kEpsilon &&
           set_detail.note_ == set_group.note_;
  };

  for (const auto& set_item : sets) {
    if (!groups.empty() && is_same_load(set_item, groups.back())) {
      groups.back().reps_list_.push_back(set_item.reps_);
    } else {
      SetGroup group;
      group.weight_ = set_item.weight_;
      group.unit_ = set_item.unit_;
      group.elastic_band_ = set_item.elastic_band_weight_;
      group.note_ = set_item.note_;
      group.reps_list_ = {set_item.reps_};
      group.volume_ = 0.0;
      group.estimated_1rm_ = 0.0;
      groups.push_back(group);
    }
  }

  // Calculate volume and max 1RM for each group
  for (auto& group : groups) {
    int max_reps = 0;
    for (int reps : group.reps_list_) {
      group.volume_ += group.weight_ * reps;
      max_reps = std::max(max_reps, reps);
    }
    if (max_reps > 1) {
      group.estimated_1rm_ = group.weight_ * (1.0 + max_reps / 30.0);
    } else {
      group.estimated_1rm_ = group.weight_;
    }
  }

  return groups;
}

auto MarkdownFormatter::FormatExercise(std::ostream& md_file,
                                       const LogEntry& log) -> void {
  md_file << "  - **" << log.exercise_name_ << "**\n";
  if (!log.project_note_.empty()) {
    md_file << "    - **Note:** " << log.project_note_ << "\n";
  }

  if (log.sets_.empty()) {
    return;
  }

  auto groups = GroupSets(log.sets_);

  constexpr double kEpsilon = 0.001;
  for (const auto& group : groups) {
    std::stringstream report_stream;
    if (group.elastic_band_ > kEpsilon) {
      report_stream << "-" << group.elastic_band_ << group.unit_;
    } else {
      report_stream << group.weight_ << group.unit_;
    }

    report_stream << " x [";
    for (size_t i = 0; i < group.reps_list_.size(); ++i) {
      report_stream << group.reps_list_[i];
      if (i < group.reps_list_.size() - 1) {
        report_stream << ", ";
      }
    }
    report_stream << "]";

    md_file << "    - `" << report_stream.str() << "`";
    md_file << " (Vol: " << std::fixed << std::setprecision(1) << group.volume_ << "kg";
    if (group.reps_list_[0] > 1 || group.reps_list_.size() > 1) {
       md_file << ", e1RM: " << group.estimated_1rm_ << "kg";
    }
    md_file << ")\n";

    if (!group.note_.empty()) {
      md_file << "      - **Note:** " << group.note_ << "\n";
    }
  }
}

auto MarkdownFormatter::ExportToMarkdown(
    const std::map<std::string, CycleData>& data_by_cycle,
    const std::vector<PRRecord>& prs,
    const std::string& output_dir) -> bool {
  std::cout << "Exporting data to Markdown files (Enhanced Reporting System)..."
            << std::endl;

  try {
    fs::create_directories(output_dir);
  } catch (const fs::filesystem_error& e) {
    std::cerr << "Error creating root directory " << output_dir << ": "
              << e.what() << std::endl;
    return false;
  }

  // Generate Global Summary (Hall of Fame)
  ExportSummary(prs, output_dir);

  for (const auto& [cycle_id, cycle_data] : data_by_cycle) {
    ProcessCycle(cycle_id, cycle_data, output_dir);
  }

  std::cout << "Markdown export completed." << std::endl;
  return true;
}

auto MarkdownFormatter::ProcessCycle(const std::string& cycle_id,
                                    const CycleData& cycle_data,
                                    const std::string& output_dir) -> void {
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
    ProcessType({.cycle_id = cycle_id, .type = type}, type_logs, cycle_data, cycle_dir);
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

  std::string display_title(params.type);
  if (!display_title.empty()) {
    display_title[0] = static_cast<char>(toupper(display_title[0]));
  }

  md_file << "# " << display_title << " Training Report\n\n";
  md_file << "**Cycle:** `" << params.cycle_id << "`\n\n";

  // Fixed Kinematics Dashboard
  md_file << "## 📊 Kinematics Dashboard\n";
  md_file << "| Metric | Value |\n";
  md_file << "| :--- | :--- |\n";
  md_file << "| **Total Volume** | " << std::fixed << std::setprecision(1) << cycle_data.total_volume_ << " kg |\n";
  md_file << "| **Avg Intensity** | " << cycle_data.average_intensity_ << " kg/rep |\n";
  md_file << "| **Frequency** | " << (cycle_data.total_days_ > 0 ? static_cast<double>(cycle_data.session_count_) / (cycle_data.total_days_ / 7.0) : 0.0) << " sessions/week |\n";
  
  auto get_percent = [&](double vol) {
    return (cycle_data.total_volume_ > 0) ? (vol / cycle_data.total_volume_ * 100.0) : 0.0;
  };
  md_file << "| **Dist. Power (1-5)** | " << get_percent(cycle_data.vol_power_) << "% |\n";
  md_file << "| **Dist. Hyper (6-12)** | " << get_percent(cycle_data.vol_hypertrophy_) << "% |\n";
  md_file << "| **Dist. Endure (13+)** | " << get_percent(cycle_data.vol_endurance_) << "% |\n\n";

  md_file << "---\n\n";

  std::map<std::string, std::vector<LogEntry>> daily_logs_for_type;
  for (const auto& log : type_logs) {
    daily_logs_for_type[log.date_].push_back(log);
  }

  for (const auto& [date, daily_entries] : daily_logs_for_type) {
    ProcessDateGroup(md_file, date, daily_entries);
  }
}

auto MarkdownFormatter::ProcessDateGroup(
    std::ofstream& md_file, const std::string& date,
    const std::vector<LogEntry>& daily_entries) -> void {
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
    FormatExercise(md_file, log);
  }
  md_file << "\n---\n\n";
}

auto MarkdownFormatter::ExportSummary(const std::vector<PRRecord>& prs,
                                      const std::string& output_dir) -> void {
  fs::path summary_path = fs::path(output_dir) / "Summary.md";
  std::ofstream md_file(summary_path);

  if (!md_file.is_open()) {
    std::cerr << "Error: Could not open summary file: " << summary_path << std::endl;
    return;
  }

  md_file << "# 🏆 Training Hall of Fame\n\n";
  md_file << "Generated on: " << __DATE__ << "\n\n";
  
  md_file << "## 🚀 Personal Records (PRs)\n";
  md_file << "| Exercise | Max Weight | Reps | Date | Est. 1RM (Epley) | Est. 1RM (Brzycki) |\n";
  md_file << "| :--- | :--- | :--- | :--- | :--- | :--- |\n";
  
  for (const auto& pr : prs) {
    md_file << "| **" << pr.exercise_name_ << "** | " << std::fixed << std::setprecision(1) << pr.max_weight_ << " kg | " 
            << pr.reps_ << " | " << pr.date_ << " | " << pr.estimated_1rm_epley_ << " kg | " 
            << pr.estimated_1rm_brzycki_ << " kg |\n";
  }

  md_file << "\n---\n*Keep pushing your limits!*";
}
