// report/formatter/MarkdownFormatter.cpp

#include "MarkdownFormatter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <map> 
#include <vector>
#include <iomanip>
#include <cmath> // 必须包含此头文件以使用 std::abs

namespace fs = std::filesystem;

// [NEW] 辅助函数实现：将 sets 聚合成 groups
std::vector<MarkdownFormatter::SetGroup> MarkdownFormatter::groupSets(const std::vector<SetDetail>& sets) {
    std::vector<SetGroup> groups;

    // Lambda: 判断是否属于同一重量配置
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

// [NEW] 辅助函数实现：格式化单个动作
void MarkdownFormatter::formatExercise(std::ostream& md_file, const LogEntry& log) {
    // 1. 打印动作名称
    md_file << "  - " << log.exercise_name << "\n";

    if (log.sets.empty()) return;

    // 2. 调用聚合逻辑
    auto groups = groupSets(log.sets);

    // 3. 打印每一组（换行缩进显示）
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

// 主函数：现在结构非常扁平，逻辑清晰
bool MarkdownFormatter::export_to_markdown(const std::map<std::string, CycleData>& data_by_cycle, const std::string& output_dir) {
    std::cout << "Exporting data to Markdown files..." << std::endl;
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory " << output_dir << ": " << e.what() << std::endl;
        return false;
    }

    // [Loop 1] 遍历每个训练周期
    for (const auto& [cycle_id, cycle_data] : data_by_cycle) {
        fs::path file_path = fs::path(output_dir) / (cycle_id + ".md");
        std::ofstream md_file(file_path);

        if (!md_file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << file_path << std::endl;
            continue;
        }

        std::cout << "  -> Writing file: " << file_path << std::endl;

        // 写入文件头
        md_file << "# " << cycle_data.type << " Training Cycle Report\n\n";
        md_file << "**Cycle ID:** `" << cycle_id << "`\n";
        md_file << "**Total Days:** " << cycle_data.total_days << "\n\n";
        md_file << "---\n\n";

        // 整理数据：按日期分组
        std::map<std::string, std::vector<LogEntry>> logs_by_date;
        for (const auto& log : cycle_data.logs) {
            logs_by_date[log.date].push_back(log);
        }

        // [Loop 2] 遍历该周期内的每一天
        for (const auto& [date, daily_logs] : logs_by_date) {
            md_file << "* **" << date << "**\n";

            // [Loop 3] 遍历当天的每一个动作
            // 注意：这里原本复杂的逻辑现在被委托给了 formatExercise
            for (const auto& log : daily_logs) {
                formatExercise(md_file, log);
            }
        }
    } 

    std::cout << "Markdown export completed." << std::endl;
    return true;
}