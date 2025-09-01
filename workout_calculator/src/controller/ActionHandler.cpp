// src/controller/ActionHandler.cpp

#include "ActionHandler.hpp"
#include "common/FileReader.hpp"
#include "common/JsonReader.hpp"
#include "reprocessor/log_formatter/JsonFormatter.hpp" 
#include "db/DbManager.hpp"
#include "db/DbInsertor.hpp"
#include "report/facade/ReportFacade.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

bool ActionHandler::run(const AppConfig& config) {
    // 根据操作类型，决定是否需要配置 reprocessor
    if (config.action == ActionType::Validate || config.action == ActionType::Convert) {
        if (!reprocessor_.configure(config.mapping_path)) {
            return false;
        }

        std::vector<std::string> filesToProcess = FileReader::findFilesByExtension(config.log_filepath, ".txt");
        if (filesToProcess.empty()) {
            std::cout << "Warning: No .txt files found to process." << std::endl;
            return true;
        }

        int successCount = 0;
        for (const auto& filePath : filesToProcess) {
            std::cout << "===== File: " << filePath << " =====" << std::endl;
            bool result = false;

            if (config.action == ActionType::Validate) {
                std::cout << "Performing validation..." << std::endl;
                if (reprocessor_.validate(filePath)) {
                    std::cout << "Validation successful." << std::endl;
                    result = true;
                } else {
                    std::cerr << "Validation failed." << std::endl;
                }
            } 
            else if (config.action == ActionType::Convert) {
                std::cout << "Performing conversion..." << std::endl;
                if (!reprocessor_.validate(filePath)) {
                    std::cerr << "Validation failed, skipping conversion." << std::endl;
                } else {
                    auto processedDataOpt = reprocessor_.convert(filePath);
                    if (processedDataOpt.has_value() && !processedDataOpt.value().empty()) {
                        try {
                            std::map<std::string, std::vector<DailyData>> dataByType;
                            for (const auto& daily : processedDataOpt.value()) {
                                for (const auto& project : daily.projects) {
                                    auto& dailyDataForType = dataByType[project.type];
                                    auto it = std::find_if(dailyDataForType.begin(), dailyDataForType.end(),
                                                        [&](const DailyData& d) { return d.date == daily.date; });
                                    if (it != dailyDataForType.end()) {
                                        it->projects.push_back(project);
                                    } else {
                                        dailyDataForType.push_back({daily.date, {project}});
                                    }
                                }
                            }
                            
                            const std::string output_dir_base = "reprocessed_json";
                            fs::path reprocessed_base_path = fs::path(config.base_path) / output_dir_base;
                            std::string base_filename = fs::path(filePath).stem().string() + "_reprocessed.json";

                            for (const auto& [type, typeData] : dataByType) {
                                fs::path type_specific_path = reprocessed_base_path / type;
                                fs::create_directories(type_specific_path);
                                fs::path output_filepath = type_specific_path / base_filename;
                                std::string outputContent = JsonFormatter::format(typeData);
                                
                                std::cout << "Writing data for type '" << type << "' to '" << output_filepath.string() << "'..." << std::endl;
                                if (!writeStringToFile(output_filepath.string(), outputContent)) {
                                    std::cerr << "Error writing file for type '" << type << "'." << std::endl;
                                }
                            }
                            result = true;
                            std::cout << "Conversion successful." << std::endl;

                        } catch (const fs::filesystem_error& e) {
                            std::cerr << "Filesystem error during output: " << e.what() << std::endl;
                        }
                    } else if (processedDataOpt.has_value()) {
                         std::cout << "Conversion resulted in no data, skipping output." << std::endl;
                         result = true;
                    } else {
                        std::cerr << "Conversion failed." << std::endl;
                    }
                }
            }

            if (result) successCount++;
            std::cout << "====================================\n" << std::endl;
        }

        std::cout << "Processing complete. " << successCount << " of " << filesToProcess.size() << " files handled successfully." << std::endl;
        return successCount == filesToProcess.size();

    } else if (config.action == ActionType::Insert) {
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
                if (DbInsertor::insert_data(dbManager.getConnection(), jsonDataOpt.value())) {
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
        std::cout << "Performing report export from database..." << std::endl;

        fs::path db_path = fs::path(config.base_path) / "workout_logs.sqlite3";
        if (!fs::exists(db_path)) {
            std::cerr << "Error: Database file not found at " << db_path.string() << std::endl;
            std::cerr << "Please run the 'insert' command first to create and populate the database." << std::endl;
            return false;
        }

        DbManager dbManager(db_path.string());
        if (!dbManager.open()) {
            return false;
        }
        
        fs::path output_dir = fs::path(config.base_path) / "output_file" / "md";
        
        // [MODIFIED] 调用 ReportFacade 的新接口
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

bool ActionHandler::writeStringToFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open output file: " << filepath << std::endl;
        return false;
    }
    file << content;
    file.close();
    return true;
}