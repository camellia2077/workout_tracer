// main_cli.cpp

#include "controller/ActionHandler.h"
#include <iostream>
#include <optional>
#include <string>
#include <filesystem>

// (新增) 引入 Windows 头文件以支持编码设置
#ifdef _WIN32
#include <windows.h>
#endif

void printUsage(const char* programName) {
    // 更新用法说明，表明 -p/--path 是可选的
    std::cerr << "Usage: " << programName << " [-p|--path] <log_file.txt> [--year <YYYY>] [--validate]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Description:" << std::endl;
    std::cerr << "  Processes or validates a workout log file. By default, it processes the file," << std::endl;
    std::cerr << "  generates a reprocessed text file, and saves the data to a local database." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -p, --path <file>      Optional: Specify the path to the workout log file. You can also provide the path directly." << std::endl;
    std::cerr << "  -y, --year <year>      Optional: Specify a year for processing. Defaults to current year." << std::endl;
    std::cerr << "  -v, --validate         Optional: Only validate the log file format and exit without processing." << std::endl;
    std::cerr << "  -h, --help             Show this help message and exit." << std::endl;
}

std::optional<AppConfig> parseCommandLine(int argc, char* argv[]) {
    if (argc == 1) {
        printUsage(argv[0]);
        return std::nullopt;
    }

    AppConfig config;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return std::nullopt;
        } else if ((arg == "-p" || arg == "--path") && i + 1 < argc) {
            config.log_filepath = argv[++i];
        } else if ((arg == "-y" || arg == "--year") && i + 1 < argc) {
            try {
                config.specified_year = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid year format provided." << std::endl;
                return std::nullopt;
            }
        } else if (arg == "-v" || arg == "--validate") {
            config.validate_only = true;
        }
        // (新逻辑) 如果参数不以'-'开头, 且我们还没有文件路径, 就把它当作文件路径
        else if (arg[0] != '-' && config.log_filepath.empty()) {
            config.log_filepath = arg;
        }
        else {
            std::cerr << "Error: Unknown or invalid argument '" << arg << "'" << std::endl;
            printUsage(argv[0]);
            return std::nullopt;
        }
    }

    if (config.log_filepath.empty()) {
        std::cerr << "Error: Log file path is required. Use --help for more information." << std::endl;
        return std::nullopt;
    }

    std::filesystem::path exe_path = argv[0];
    config.db_path = (exe_path.parent_path() / "workouts.sqlite3").string();
    config.mapping_path = "mapping.json";

    return config;
}

int main(int argc, char* argv[]) {
    // (新增) 设置 Windows 控制台的输入和输出编码为 UTF-8
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    auto configOpt = parseCommandLine(argc, argv);
    if (!configOpt.has_value()) {
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") return 0;
        }
        return 1;
    }

    ActionHandler handler;
    bool success = handler.run(configOpt.value());

    return success ? 0 : 1;
}