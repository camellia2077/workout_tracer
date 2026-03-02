#include "ActionHandler.h"
#include "common/TxtFileReader.h" // **新增**: 引入 TxtFileReader 头文件
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

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

    // **核心改动**: 直接调用 TxtFileReader 来获取所有需要处理的文件路径
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

/**
 * @brief 处理单个日志文件的核心逻辑 (此函数保持不变)
 */
bool ActionHandler::processFile(const std::string& logFilePath, const AppConfig& config) {
    // 1. 验证文件
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

    // 2. 解析文件
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

    // 3. 处理数据
    std::cout << "Processing extracted data..." << std::endl;
    reprocessor_.processData(processedData, config.specified_year);
    std::cout << "Data processed successfully." << std::endl;

    // 4. 根据模式决定是否将格式化后的字符串写入文件
    if (config.output_mode == OutputMode::ALL || config.output_mode == OutputMode::FILE_ONLY) {
        std::string outputContent = reprocessor_.formatDataToString(processedData);
        try {
            const std::string output_dir = "reprocessed";
            fs::path test_dir_base(config.db_path);
            fs::path reprocessed_path = test_dir_base.parent_path() / output_dir;
            fs::create_directories(reprocessed_path); // 使用 create_directories 更安全
            
            fs::path input_path(logFilePath);
            std::string output_filename = input_path.stem().string() + "_reprocessed.txt";
            fs::path output_filepath = reprocessed_path / output_filename;

            std::cout << "Writing processed data to '" << output_filepath.string() << "'..." << std::endl;
            if (!writeStringToFile(output_filepath.string(), outputContent)) {
                return false;
            }
            std::cout << "Successfully wrote data to file." << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            return false;
        }
    }

    // 5. 根据模式决定是否保存到数据库
    if (config.output_mode == OutputMode::ALL || config.output_mode == OutputMode::DB_ONLY) {
        std::cout << "Database sync process started..." << std::endl;
        if (!dataManager_.connectAndInitialize(config.db_path)) {
            std::cerr << "Error: Database setup failed for: " << config.db_path << std::endl;
            return false;
        }
        if (!dataManager_.saveData(processedData)) {
            std::cerr << "Error: Failed to save data to the database." << std::endl;
            return false;
        }
        std::cout << "Data successfully saved to database: " << config.db_path << std::endl;
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