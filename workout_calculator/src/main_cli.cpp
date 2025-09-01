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
    std::cerr << "Usage: " << programName << " <path> <command> [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Description:" << std::endl;
    std::cerr << "  Processes or validates workout logs. The <path> can be a single .txt file" << std::endl;
    std::cerr << "  or a directory containing .txt files." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Commands:" << std::endl;
    std::cerr << "  validate               Only validate the log file format and exit." << std::endl;
    std::cerr << "  convert                Convert the log file to JSON format." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options for 'convert':" << std::endl;
    std::cerr << "  -y, --year <year>      Specify a 4-digit year for processing. Defaults to the current year." << std::endl;
    std::cerr << std::endl;
    std::cerr << "General Options:" << std::endl;
    std::cerr << "  -h, --help             Show this help message and exit." << std::endl;
}

std::optional<AppConfig> parseCommandLine(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return std::nullopt;
    }

    AppConfig config;
    std::vector<std::string> args(argv + 1, argv + argc);

    config.log_filepath = args[0];
    std::string command = args[1];

    if (command == "validate") {
        config.action = ActionType::Validate;
    } else if (command == "convert") {
        config.action = ActionType::Convert;
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage(argv[0]);
        return std::nullopt;
    }

    // Parse options
    for (size_t i = 2; i < args.size(); ++i) {
        if (args[i] == "-h" || args[i] == "--help") {
            printUsage(argv[0]);
            return std::nullopt; 
        } else if ((args[i] == "-y" || args[i] == "--year") && i + 1 < args.size()) {
            if (config.action != ActionType::Convert) {
                std::cerr << "Error: --year option is only valid for the 'convert' command." << std::endl;
                return std::nullopt;
            }
            try {
                config.specified_year = std::stoi(args[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid year format provided." << std::endl;
                return std::nullopt;
            }
        } else {
            std::cerr << "Error: Unknown or invalid argument '" << args[i] << "'" << std::endl;
            printUsage(argv[0]);
            return std::nullopt;
        }
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