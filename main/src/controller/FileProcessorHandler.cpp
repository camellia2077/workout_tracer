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
                
                // [MODIFIED] 核心修改：不再进行拆分，直接处理整个文件
                if (processedDataOpt.has_value() && !processedDataOpt.value().empty()) {
                    try {
                        const std::string output_dir_base = "reprocessed_json";
                        fs::path reprocessed_base_path = fs::path(config.base_path) / output_dir_base;
                        
                        // 确保目录存在
                        fs::create_directories(reprocessed_base_path);

                        // 生成单一的文件名 (例如: 2025_7.json)
                        std::string base_filename = fs::path(filePath).stem().string() + ".json";
                        fs::path output_filepath = reprocessed_base_path / base_filename;
                        
                        // 直接格式化整个 processedData
                        std::string outputContent = JsonFormatter::format(processedDataOpt.value());
                        
                        std::cout << "Writing converted data to '" << output_filepath.string() << "'..." << std::endl;
                        if (writeStringToFile(output_filepath.string(), outputContent)) {
                            result = true;
                            std::cout << "Conversion successful." << std::endl;
                        } else {
                            std::cerr << "Error writing file." << std::endl;
                        }

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