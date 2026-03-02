// main.cpp

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include <filesystem>

// 包含总控类和数据库管理器的头文件
#include "reprocessor/Reprocessor.h"
#include "db_inserter/SqliteManager.h" 

/**
 * @brief 将一个字符串内容写入指定的文件。
 */
bool writeStringToFile(const std::string& output_filepath, const std::string& content) {
    std::ofstream file(output_filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open output file: " << output_filepath << std::endl;
        return false;
    }
    file << content;
    file.close();
    return true;
}

/**
 * @brief 打印更新后的使用说明
 */
void printUsage(const char* programName) {
    // <-- 修改：简化了用法说明
    std::cerr << "Usage: " << programName << " --path <log_file.txt> [--year <YYYY>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Description:" << std::endl;
    std::cerr << "  Processes a workout log file, generates a reprocessed text file," << std::endl;
    std::cerr << "  and automatically saves the data to 'workouts.sqlite' in the program's directory." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -p, --path <file>      Required: Path to the workout log file." << std::endl;
    std::cerr << "  -y, --year <year>      Optional: Specify a year for processing. Defaults to current year." << std::endl;
    std::cerr << "  -h, --help             Show this help message and exit." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printUsage(argv[0]);
        return 0;
    }

    // --- 1. 解析命令行参数 (已简化) ---
    std::string log_filepath;
    std::optional<int> specified_year;
    const std::string mapping_filename = "mapping.json";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } 
        else if ((arg == "-p" || arg == "--path") && i + 1 < argc) {
            log_filepath = argv[++i];
        } else if ((arg == "-y" || arg == "--year") && i + 1 < argc) {
            try {
                specified_year = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid year format provided." << std::endl;
                return 1;
            }
        }
        else {
            std::cerr << "Error: Unknown or invalid argument '" << arg << "'" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    if (log_filepath.empty()) {
        std::cerr << "Error: Log file path is required. Use --help for more information." << std::endl;
        return 1;
    }

    // --- 2. 动态获取程序路径并构建数据库路径  ---
    std::filesystem::path exe_path = argv[0];
    std::filesystem::path db_path = exe_path.parent_path() / "workouts.sqlite3";

    // 3. 配置 Reprocessor
    Reprocessor reprocessor;
    std::cout << "Configuring reprocessor with '" << mapping_filename << "'..." << std::endl;
    if (!reprocessor.configure(mapping_filename)) { return 1; }
    std::cout << "Reprocessor configured successfully." << std::endl;
    
    // 4. 处理日志文件
    std::cout << "Processing log file '" << log_filepath << "'..." << std::endl;
    std::vector<DailyData> processedData = reprocessor.processLogFile(log_filepath, specified_year);
    if (processedData.empty()) {
        std::cout << "Warning: No data was processed from the log file. Exiting." << std::endl;
        return 0;
    }
    std::cout << "Log file processed successfully." << std::endl;
    
    // 5. 将格式化后的字符串写入文件
    std::string outputContent = reprocessor.formatDataToString(processedData);
    try {
        const std::string output_dir = "reprocessed";
        std::filesystem::create_directory(output_dir);
        std::filesystem::path input_path(log_filepath);
        std::string output_filename = input_path.stem().u8string() + "_reprocessed.txt";
        std::filesystem::path output_filepath = std::filesystem::path(output_dir) / output_filename;

        std::cout << "Writing processed data to '" << output_filepath.string() << "'..." << std::endl;
        if (!writeStringToFile(output_filepath.string(), outputContent)) { return 1; }
        std::cout << "Successfully wrote data to file." << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return 1;
    }

    // --- 步骤 6: 总是尝试保存到硬编码的数据库路径 ---
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Database sync process started..." << std::endl;

    SqliteManager dbManager;
    
    // 使用我们构建好的 db_path
    if (!dbManager.connect(db_path.string())) {
        std::cerr << "Error: Database connection failed for: " << db_path.string() << std::endl;
        return 1;
    }

    if (dbManager.saveData(processedData)) {
        std::cout << "Data successfully saved to database: " << db_path.string() << std::endl;
    } else {
        std::cerr << "Error: Failed to save data to the database." << std::endl;
        return 1;
    }

    return 0;
}