// src/controller/ActionHandler.cpp

#include "ActionHandler.hpp"
#include "common/TxtFileReader.hpp"
// [MODIFIED] 引入新的 JsonFormatter
#include "reprocessor/log_formatter/JsonFormatter.hpp" 
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

/**
 * @brief 主运行函数，现在委托 TxtFileReader 来获取文件列表。
 */
bool ActionHandler::run(const AppConfig& config) {
    // 1. 配置 Reprocessor
    std::cout << "Configuring reprocessor with '" << config.mapping_path << "'..." << std::endl;
    if (!reprocessor_.configure(config.mapping_path)) {
        return false;
    }
    std::cout << "Reprocessor configured successfully." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 无论是单个文件还是目录，TxtFileReader 都会正确处理
    std::vector<std::string> filesToProcess = TxtFileReader::getTxtFilePaths(config.log_filepath);

    if (filesToProcess.empty()) {
        std::cout << "Warning: No .txt files found to process at the specified path." << std::endl;
        return true;
    }

    // 3. 遍历并处理所有找到的文件
    int successCount = 0;
    for (const auto& filePath : filesToProcess) {
        std::cout << "===== Processing file: " << filePath << " =====" << std::endl;
        if (processFile(filePath, config)) {
            successCount++;
            std::cout << "===== Successfully processed file: " << filePath << " =====\n" << std::endl;
        } else {
            std::cerr << "===== Failed to process file: " << filePath << " =====\n" << std::endl;
        }
    }
    
    // 4. 返回最终结果
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Processing complete. " << successCount << " of " << filesToProcess.size() << " files processed successfully." << std::endl;
    return successCount == filesToProcess.size();
}

bool ActionHandler::processFile(const std::string& logFilePath, const AppConfig& config) {
    // 1. 验证文件 (不变)
    std::cout << "Validating log file..." << std::endl;
    if (!reprocessor_.validateFile(logFilePath, config.mapping_path)) {
        std::cerr << "Error: Log file validation failed. Please check the file format." << std::endl;
        return false;
    }
    std::cout << "Validation successful." << std::endl;

    if (config.validate_only) {
        std::cout << "Validation-only mode. Exiting file processing." << std::endl;
        return true;
    }

    // 2. 解析文件 (不变)
    std::cout << "Parsing log file..." << std::endl;
    auto parsedDataOpt = reprocessor_.parseFile(logFilePath);
    if (!parsedDataOpt.has_value()) {
        std::cerr << "Error: File parsing failed after successful validation." << std::endl;
        return false;
    }
    std::vector<DailyData> processedData = parsedDataOpt.value();
    if (processedData.empty()) {
        std::cout << "Warning: No data was found in the log file. Exiting." << std::endl;
        return true;
    }
    std::cout << "File parsed successfully." << std::endl;

    // 3. 处理数据 (不变)
    std::cout << "Processing extracted data..." << std::endl;
    reprocessor_.processData(processedData, config.specified_year);
    std::cout << "Data processed successfully." << std::endl;

    // 4. 将格式化后的字符串写入文件 (逻辑简化)
    std::cout << "Grouping data by type and writing to separate JSON files..." << std::endl;

    // 按类型对数据进行分组 (不变)
    std::map<std::string, std::vector<DailyData>> dataByType;
    for (const auto& daily : processedData) {
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

    // 写入文件到对应子文件夹 (路径逻辑修改)
    try {
        const std::string output_dir_base = "reprocessed_json";
        fs::path reprocessed_base_path = fs::path(config.base_path) / output_dir_base;

        fs::path input_path(logFilePath);
        // [MODIFIED] 修改输出文件名和扩展名
        std::string base_filename = input_path.stem().string() + "_reprocessed.json";

        for (const auto& [type, typeData] : dataByType) {
            if (typeData.empty()) continue;

            fs::path type_specific_path = reprocessed_base_path / type;
            fs::create_directories(type_specific_path);

            fs::path output_filepath = type_specific_path / base_filename;
            
            // [MODIFIED] 调用新的 JsonFormatter
            std::string outputContent = JsonFormatter::format(typeData);

            std::cout << "Writing data for type '" << type << "' to '" << output_filepath.string() << "'..." << std::endl;
            if (!writeStringToFile(output_filepath.string(), outputContent)) {
                 std::cerr << "Error: Failed to write file for type '" << type << "'." << std::endl;
            }
        }
        std::cout << "Successfully wrote data to type-specific directories." << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }

    return true;
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