// controller/FileProcessorHandler.cpp

#include "FileProcessorHandler.hpp"
#include "common/FileReader.hpp"
#include "reprocessor/preprocessor/log_formatter/JsonFormatter.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

bool FileProcessorHandler::handle(const AppConfig& config) {
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
                        
                        fs::create_directories(reprocessed_base_path);

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
        std::cout << "====================================\\n" << std::endl;
    }

    std::cout << "Processing complete. " << successCount << " of " << filesToProcess.size() << " files handled successfully." << std::endl;
    return successCount == filesToProcess.size();
}

bool FileProcessorHandler::writeStringToFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open output file: " << filepath << std::endl;
        return false;
    }
    file << content;
    file.close();
    return true;
}