// src/main_cli.cpp

#include "controller/ActionHandler.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <command> [<path>] [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Description:" << std::endl;
    std::cerr << "  Processes, validates, inserts, or exports workout logs." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Commands:" << std::endl;
    std::cerr << "  validate <path>        Only validate the log file format." << std::endl;
    std::cerr << "  convert <path>         Convert the log file to JSON format." << std::endl;
    std::cerr << "  insert <path>          Insert JSON files into the database." << std::endl;
    // [NEW] 添加了 export 命令
    std::cerr << "  export                 Export all data from the database to Markdown files." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -y, --year <year>      (For 'convert') Specify a 4-digit year." << std::endl;
    std::cerr << "  -h, --help             Show this help message and exit." << std::endl;
}

std::optional<AppConfig> parseCommandLine(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return std::nullopt;
    }

    AppConfig config;
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string command = args[0];

    if (command == "export") {
        config.action = ActionType::Export;
        if (args.size() > 1) { // export 命令不应有额外参数
            std::cerr << "Error: 'export' command does not take any additional arguments." << std::endl;
            return std::nullopt;
        }
    } 
    else if (command == "validate" || command == "convert" || command == "insert") {
        if (args.size() < 2) {
            std::cerr << "Error: '" << command << "' command requires a <path> argument." << std::endl;
            return std::nullopt;
        }
        config.log_filepath = args[1];
        if (command == "validate") config.action = ActionType::Validate;
        if (command == "convert") config.action = ActionType::Convert;
        if (command == "insert") config.action = ActionType::Insert;

        // 解析选项
        for (size_t i = 2; i < args.size(); ++i) {
            if ((args[i] == "-y" || args[i] == "--year") && i + 1 < args.size()) {
                if (config.action != ActionType::Convert) {
                    std::cerr << "Error: --year option is only valid for the 'convert' command." << std::endl;
                    return std::nullopt;
                }
                config.specified_year = std::stoi(args[++i]);
            } else {
                std::cerr << "Error: Unknown or invalid argument '" << args[i] << "'" << std::endl;
                return std::nullopt;
            }
        }
    } else if (command == "-h" || command == "--help") {
        printUsage(argv[0]);
        return std::nullopt;
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage(argv[0]);
        return std::nullopt;
    }

    std::filesystem::path exe_path = argv[0];
    config.base_path = exe_path.parent_path().string();
    config.mapping_path = "mapping.json";

    return config;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    auto configOpt = parseCommandLine(argc, argv);
    if (!configOpt.has_value()) {
        return 1;
    }

    ActionHandler handler;
    bool success = handler.run(configOpt.value());

    return success ? 0 : 1;
}