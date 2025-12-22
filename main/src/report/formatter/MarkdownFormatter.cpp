// src/report/formatter/MarkdownFormatter.cpp

#include "MarkdownFormatter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <map> 
#include <vector>
#include <iomanip>
#include <cmath>
#include <algorithm> 

namespace fs = std::filesystem;

// -------------------------------------------------------------------------
// 1. 辅助函数实现：将 sets 聚合成 groups
// -------------------------------------------------------------------------
std::vector<MarkdownFormatter::SetGroup> MarkdownFormatter::groupSets(const std::vector<SetDetail>& sets) {
    std::vector<SetGroup> groups;

    auto is_same_load = [](const SetDetail& a, const SetGroup& b) {
        return std::abs(a.weight - b.weight) < 0.001 &&
               a.unit == b.unit &&
               std::abs(a.elastic_band_weight - b.elastic_band) < 0.001;
    };

    for (const auto& set : sets) {
        if (!groups.empty() && is_same_load(set, groups.back())) {
            // 如果和最后一组配置相同，只添加次数
            groups.back().reps_list.push_back(set.reps);
        } else {
            // 否则开启新的一组
            groups.push_back({set.weight, set.unit, set.elastic_band_weight, {set.reps}});
        }
    }
    return groups;
}

// -------------------------------------------------------------------------
// 2. 辅助函数实现：格式化并输出单个动作
// -------------------------------------------------------------------------
void MarkdownFormatter::formatExercise(std::ostream& md_file, const LogEntry& log) {
    // 动作名称作为列表项
    md_file << "  - **" << log.exercise_name << "**\n"; 

    if (log.sets.empty()) return;

    auto groups = groupSets(log.sets);

    for (const auto& group : groups) {
        std::stringstream ss;
        // 格式化重量部分
        if (group.elastic_band > 0.001) {
            ss << "-" << group.elastic_band << group.unit; // 弹力带辅助显示为负数
        } else {
            ss << group.weight << group.unit;
        }
        
        // 格式化次数部分，例如 x [10, 10, 8]
        ss << " x [";
        for (size_t i = 0; i < group.reps_list.size(); ++i) {
            ss << group.reps_list[i];
            if (i < group.reps_list.size() - 1) ss << ", ";
        }
        ss << "]";

        // 输出为子列表项
        md_file << "    - `" << ss.str() << "`\n";
    }
}

// -------------------------------------------------------------------------
// 3. 主导出函数实现
// -------------------------------------------------------------------------
bool MarkdownFormatter::export_to_markdown(const std::map<std::string, CycleData>& data_by_cycle, const std::string& output_dir) {
    std::cout << "Exporting data to Markdown files (Directory Structure)..." << std::endl;
    
    // 创建根输出目录
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating root directory " << output_dir << ": " << e.what() << std::endl;
        return false;
    }

    // [Loop 1] 遍历每个训练周期 (例如: 2025-07-05)
    for (const auto& [cycle_id, cycle_data] : data_by_cycle) {
        
        // 1. 为该周期创建一个专属文件夹
        fs::path cycle_dir = fs::path(output_dir) / cycle_id;
        try {
            fs::create_directories(cycle_dir);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating cycle directory " << cycle_dir << ": " << e.what() << std::endl;
            continue;
        }

        std::cout << "  -> Processing Cycle: " << cycle_id << " into folder: " << cycle_dir << std::endl;

        // 2. 将该周期的所有日志按“类型”分组 (Push, Pull, Squat)
        std::map<std::string, std::vector<LogEntry>> logs_by_type;
        for (const auto& log : cycle_data.logs) {
            // 如果类型为空，归类为 "unknown"
            std::string type_key = log.exercise_type.empty() ? "unknown" : log.exercise_type;
            logs_by_type[type_key].push_back(log);
        }

        // [Loop 2] 遍历每种类型，生成独立文件 (例如: push.md)
        for (const auto& [type, type_logs] : logs_by_type) {
            // 文件名: push.md, pull.md
            fs::path file_path = cycle_dir / (type + ".md");
            std::ofstream md_file(file_path);

            if (!md_file.is_open()) {
                std::cerr << "Error: Could not open file: " << file_path << std::endl;
                continue;
            }

            // 首字母大写用于标题显示
            std::string display_title = type;
            if (!display_title.empty()) display_title[0] = toupper(display_title[0]);

            // 写入文件头
            md_file << "# " << display_title << " Training Report\n\n";
            md_file << "**Cycle:** `" << cycle_id << "`\n\n";
            md_file << "---\n\n";

            // 3. 在该类型文件中，按“日期”再次分组，以便按时间顺序展示
            std::map<std::string, std::vector<LogEntry>> daily_logs_for_type;
            for (const auto& log : type_logs) {
                daily_logs_for_type[log.date].push_back(log);
            }

            // [Loop 3] 按日期输出动作
            for (const auto& [date, daily_entries] : daily_logs_for_type) {
                md_file << "## " << date << "\n\n";
                
                for (const auto& log : daily_entries) {
                    formatExercise(md_file, log);
                }
                md_file << "\n---\n\n";
            }
        }
    } 

    std::cout << "Markdown export completed." << std::endl;
    return true;
}