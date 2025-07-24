#include "ActionHandler.h"
#include <iostream>
#include <fstream>
#include <filesystem>

bool ActionHandler::run(const AppConfig& config) {
    // ... (前序步骤：配置Reprocessor, 验证, 解析, 处理数据 - 保持不变)
    // 1. 配置 Reprocessor
    std::cout << "Configuring reprocessor with '" << config.mapping_path << "'..." << std::endl;
    if (!reprocessor_.configure(config.mapping_path)) { return false; }
    std::cout << "Reprocessor configured successfully." << std::endl;

    // 2. 验证文件
    std::cout << "Validating log file '" << config.log_filepath << "'..." << std::endl;
    if (!reprocessor_.validateFile(config.log_filepath, config.mapping_path)) {
        std::cerr << "Error: Log file validation failed. Please check the file format." << std::endl;
        return false;
    }
    std::cout << "Validation successful." << std::endl;

    if (config.validate_only) {
        std::cout << "Validation-only mode. Exiting." << std::endl;
        return true;
    }
    
    // --- 进入完整处理流程 ---
    std::cout << "----------------------------------------" << std::endl;
    
    // 3. 解析文件
    std::cout << "Parsing log file..." << std::endl;
    auto parsedDataOpt = reprocessor_.parseFile(config.log_filepath);
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

    // 4. 处理数据
    std::cout << "Processing extracted data..." << std::endl;
    reprocessor_.processData(processedData, config.specified_year);
    std::cout << "Data processed successfully." << std::endl;


    // --- 解耦后的输出逻辑 ---

    // 5. 根据模式决定是否将格式化后的字符串写入文件
    if (config.output_mode == OutputMode::ALL || config.output_mode == OutputMode::FILE_ONLY) {
        std::string outputContent = reprocessor_.formatDataToString(processedData);
        try {
            const std::string output_dir = "reprocessed";
            std::filesystem::create_directory(output_dir);
            std::filesystem::path input_path(config.log_filepath);
            std::string output_filename = input_path.stem().string() + "_reprocessed.txt";
            std::filesystem::path output_filepath = std::filesystem::path(output_dir) / output_filename;

            std::cout << "Writing processed data to '" << output_filepath.string() << "'..." << std::endl;
            if (!writeStringToFile(output_filepath.string(), outputContent)) { return false; }
            std::cout << "Successfully wrote data to file." << std::endl;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            return false;
        }
    }

    // 6. 根据模式决定是否保存到数据库
    if (config.output_mode == OutputMode::ALL || config.output_mode == OutputMode::DB_ONLY) {
        std::cout << "----------------------------------------" << std::endl;
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