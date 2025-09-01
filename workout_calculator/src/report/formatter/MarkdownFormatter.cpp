// src/formatter/MarkdownFormatter.cpp

#include "MarkdownFormatter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

bool MarkdownFormatter::export_to_markdown(const std::map<std::string, std::vector<LogEntry>>& data_by_type, const std::string& output_dir) {
    std::cout << "Exporting data to Markdown files..." << std::endl;
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory " << output_dir << ": " << e.what() << std::endl;
        return false;
    }

    for (const auto& [type, logs] : data_by_type) {
        fs::path file_path = fs::path(output_dir) / (type + ".md");
        std::ofstream md_file(file_path);

        if (!md_file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << file_path << std::endl;
            continue;
        }

        std::cout << "  -> Writing file: " << file_path << std::endl;

        md_file << "# " << type << " Training Logs\n\n";

        for (const auto& log : logs) {
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