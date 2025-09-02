// src/formatter/MarkdownFormatter.cpp

#include "MarkdownFormatter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

// [MODIFIED] 重写此函数以按 cycle_id 生成文件
bool MarkdownFormatter::export_to_markdown(const std::map<std::string, CycleData>& data_by_cycle, const std::string& output_dir) {
    std::cout << "Exporting data to Markdown files..." << std::endl;
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory " << output_dir << ": " << e.what() << std::endl;
        return false;
    }

    for (const auto& [cycle_id, cycle_data] : data_by_cycle) {
        // 使用 cycle_id 作为文件名
        fs::path file_path = fs::path(output_dir) / (cycle_id + ".md");
        std::ofstream md_file(file_path);

        if (!md_file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << file_path << std::endl;
            continue;
        }

        std::cout << "  -> Writing file: " << file_path << std::endl;

        // 写入报告标题，包含类型和总天数
        md_file << "# " << cycle_data.type << " Training Cycle Report\n\n";
        md_file << "**Cycle ID:** `" << cycle_id << "`\n";
        md_file << "**Total Days:** " << cycle_data.total_days << "\n\n";
        md_file << "---\n\n";

        for (const auto& log : cycle_data.logs) {
            std::stringstream reps_ss;
            reps_ss << "[";
            for (size_t i = 0; i < log.reps.size(); ++i) {
                reps_ss << log.reps[i] << (i < log.reps.size() - 1 ? ", " : "");
            }
            reps_ss << "]";

            md_file << "* **" << log.date << "** - " << log.exercise_name << " `" << reps_ss.str() << "`\n";
        }
    }
    
    std::cout << "Markdown export completed." << std::endl;
    return true;
}