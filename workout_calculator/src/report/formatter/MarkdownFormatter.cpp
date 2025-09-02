// src/formatter/MarkdownFormatter.cpp

#include "MarkdownFormatter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <map> // [ADD] 需要 map 来按日期对日志进行分组
#include <vector> // [ADD] 需要 vector

namespace fs = std::filesystem;

// [MODIFIED] 重写此函数以实现按日期分组的输出
bool MarkdownFormatter::export_to_markdown(const std::map<std::string, CycleData>& data_by_cycle, const std::string& output_dir) {
    std::cout << "Exporting data to Markdown files..." << std::endl;
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory " << output_dir << ": " << e.what() << std::endl;
        return false;
    }

    for (const auto& [cycle_id, cycle_data] : data_by_cycle) {
        fs::path file_path = fs::path(output_dir) / (cycle_id + ".md");
        std::ofstream md_file(file_path);

        if (!md_file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << file_path << std::endl;
            continue;
        }

        std::cout << "  -> Writing file: " << file_path << std::endl;

        // 写入报告标题 (保持不变)
        md_file << "# " << cycle_data.type << " Training Cycle Report\n\n";
        md_file << "**Cycle ID:** `" << cycle_id << "`\n";
        md_file << "**Total Days:** " << cycle_data.total_days << "\n\n";
        md_file << "---\n\n";

        // --- [START] 核心修改逻辑 ---

        // 1. 按日期对日志进行分组
        std::map<std::string, std::vector<LogEntry>> logs_by_date;
        for (const auto& log : cycle_data.logs) {
            logs_by_date[log.date].push_back(log);
        }

        // 2. 遍历分组后的日志并生成Markdown
        for (const auto& [date, daily_logs] : logs_by_date) {
            // 首先，打印作为主列表项的日期
            md_file << "* **" << date << "**\n";

            // 然后，遍历当天的所有训练记录
            for (const auto& log : daily_logs) {
                // 拼接次数的字符串
                std::stringstream reps_ss;
                reps_ss << "[";
                for (size_t i = 0; i < log.reps.size(); ++i) {
                    reps_ss << log.reps[i] << (i < log.reps.size() - 1 ? ", " : "");
                }
                reps_ss << "]";

                // 将训练项目作为次级列表项输出，注意前面的缩进
                md_file << "  - " << log.exercise_name << " `" << reps_ss.str() << "`\n";
            }
        }
        // --- [END] 核心修改逻辑 ---
    }
    
    std::cout << "Markdown export completed." << std::endl;
    return true;
}