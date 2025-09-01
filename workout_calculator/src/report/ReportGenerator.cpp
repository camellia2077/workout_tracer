// src/report/ReportGenerator.cpp

#include "ReportGenerator.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

// --- Public Interface ---

bool ReportGenerator::generate_markdown_files(sqlite3* db, const std::string& output_dir) {
    std::cout << "Starting report generation..." << std::endl;
    
    // 步骤 1: 从数据库查询数据
    auto data = query_all_logs(db);
    if (data.empty()) {
        std::cout << "No data found in the database to export." << std::endl;
        return true; // 没有数据也视为成功
    }

    // 步骤 2: 将查询到的数据导出为 Markdown
    return export_to_markdown(data, output_dir);
}


// --- Private Implementation ---

std::map<std::string, std::vector<ReportGenerator::LogEntry>> ReportGenerator::query_all_logs(sqlite3* db) {
    std::cout << "Querying data from database..." << std::endl;
    std::map<std::string, std::vector<LogEntry>> data_by_type;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT l.id, l.date, l.exercise_name, l.exercise_type, s.reps "
                      "FROM training_logs l "
                      "JOIN training_sets s ON l.id = s.log_id "
                      "ORDER BY l.date, l.id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare query statement: " << sqlite3_errmsg(db) << std::endl;
        return data_by_type;
    }

    std::map<long long, LogEntry> temp_logs;
    std::map<long long, std::string> log_id_to_type;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long log_id = sqlite3_column_int64(stmt, 0);
        
        if (temp_logs.find(log_id) == temp_logs.end()) {
            LogEntry entry;
            entry.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            entry.exercise_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            temp_logs[log_id] = entry;
            log_id_to_type[log_id] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        }
        
        temp_logs[log_id].reps.push_back(sqlite3_column_int(stmt, 4));
    }

    for(auto const& [log_id, entry] : temp_logs) {
        const std::string& type = log_id_to_type[log_id];
        data_by_type[type].push_back(entry);
    }

    sqlite3_finalize(stmt);
    std::cout << "Data queried successfully." << std::endl;
    return data_by_type;
}

bool ReportGenerator::export_to_markdown(const std::map<std::string, std::vector<ReportGenerator::LogEntry>>& data_by_type, const std::string& output_dir) {
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