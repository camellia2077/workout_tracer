// src/facade/ReportFacade.cpp

#include "ReportFacade.hpp"
#include "report/database/DatabaseManager.hpp"
#include "report/formatter/MarkdownFormatter.hpp"
#include <iostream>

bool ReportFacade::generate_report(sqlite3* db, const std::string& output_dir) {
    std::cout << "Starting report generation..." << std::endl;
    
    // 步骤 1: 从数据库查询数据
    auto data = DatabaseManager::query_all_logs(db);
    if (data.empty()) {
        std::cout << "No data found in the database to export." << std::endl;
        return true; // 没有数据也视为成功
    }

    // 步骤 2: 将查询到的数据导出为 Markdown
    return MarkdownFormatter::export_to_markdown(data, output_dir);
}