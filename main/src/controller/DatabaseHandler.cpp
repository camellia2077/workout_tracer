// controller/DatabaseHandler.cpp

#include "DatabaseHandler.hpp"
#include "common/FileReader.hpp"
#include "common/JsonReader.hpp"
#include "db/manager/DbManager.hpp"
#include "db/facade/DbFacade.hpp"
#include "report/facade/ReportFacade.hpp"
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

bool DatabaseHandler::handle(const AppConfig& config) {
    if (config.action == ActionType::Insert) {
        std::cout << "Performing database insertion..." << std::endl;
        
        fs::path db_path = fs::path(config.base_path) / "workout_logs.sqlite3";
        DbManager dbManager(db_path.string());

        if (!dbManager.open()) {
            return false;
        }
        
        std::vector<std::string> jsonFiles = FileReader::findFilesByExtension(config.log_filepath, ".json");
        if (jsonFiles.empty()) {
            std::cout << "Warning: No .json files found to insert." << std::endl;
            return true;
        }
        
        int successCount = 0;
        for (const auto& jsonPath : jsonFiles) {
            std::cout << "--- Inserting file: " << jsonPath << " ---" << std::endl;
            auto jsonDataOpt = JsonReader::readFile(jsonPath);
            if (jsonDataOpt.has_value()) {
                // [MODIFIED] 使用 .get() 获取原始 cJSON* 指针传递给 DbFacade
                // jsonDataOpt.value() 是 std::unique_ptr<cJSON>
                if (DbFacade::insertTrainingData(dbManager.getConnection(), jsonDataOpt.value().get())) {
                    std::cout << "Successfully inserted data from " << jsonPath << std::endl;
                    successCount++;
                } else {
                    std::cerr << "Failed to insert data from " << jsonPath << std::endl;
                }
            }
        }
        std::cout << "\nDatabase insertion complete. " << successCount << " of " << jsonFiles.size() << " files inserted successfully." << std::endl;
        return successCount == jsonFiles.size();

    } else if (config.action == ActionType::Export) {
        // ... (Export 部分代码保持不变) ...
        std::cout << "Performing report export from database..." << std::endl;

        fs::path db_path = fs::path(config.base_path) / "workout_logs.sqlite3";
        if (!fs::exists(db_path)) {
            std::cerr << "Error: Database file not found at " << db_path.string() << std::endl;
            return false;
        }

        DbManager dbManager(db_path.string());
        if (!dbManager.open()) {
            return false;
        }
        
        fs::path output_dir = fs::path(config.base_path) / "output_file" / "md";
        
        if (ReportFacade::generate_report(dbManager.getConnection(), output_dir.string())) {
            std::cout << "\nReport export completed successfully." << std::endl;
            return true;
        } else {
            std::cerr << "\nReport export failed." << std::endl;
            return false;
        }
    }
    
    return false;
}