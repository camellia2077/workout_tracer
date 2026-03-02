// main_cli.cpp

#include "controller/ActionHandler.h"
#include <iostream>
#include <optional>
#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief (已修改) 更新用法说明以反映路径可以是文件或目录。
 */
void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <path> [mode] [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Description:" << std::endl;
    std::cerr << "  Processes or validates workout logs. The <path> can be a single .txt file" << std::endl;
    std::cerr << "  or a directory containing .txt files." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Modes (mutually exclusive):" << std::endl;
    std::cerr << "  -r, --reprocess        Reprocess the log, output a formatted file, AND save to the database. (Default)" << std::endl;
    std::cerr << "  -o, --output           Reprocess the log and ONLY output the formatted file." << std::endl;
    std::cerr << "  -p, --persist          Reprocess the log and ONLY save the data to the database." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -y, --year <year>      Specify a 4-digit year for processing. Defaults to the current year." << std::endl;
    std::cerr << "  -v, --validate         Only validate the log file format and exit. Overrides any mode." << std::endl;
    std::cerr << "  -h, --help             Show this help message and exit." << std::endl;
}

std::optional<AppConfig> parseCommandLine(int argc, char* argv[]) {
    if (argc == 1) {
        printUsage(argv[0]);
        return std::nullopt;
    }

    AppConfig config;
    std::optional<OutputMode> specifiedMode;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return std::nullopt;
        }
        else if (arg == "-r" || arg == "--reprocess") {
            if (specifiedMode.has_value()) {
                std::cerr << "Error: Mode flags (-r, -o, -p) are mutually exclusive." << std::endl;
                return std::nullopt;
            }
            specifiedMode = OutputMode::ALL;
        }
        else if (arg == "-o" || arg == "--output") {
            if (specifiedMode.has_value()) {
                std::cerr << "Error: Mode flags (-r, -o, -p) are mutually exclusive." << std::endl;
                return std::nullopt;
            }
            specifiedMode = OutputMode::FILE_ONLY;
        }
        else if (arg == "-p" || arg == "--persist") {
            if (specifiedMode.has_value()) {
                std::cerr << "Error: Mode flags (-r, -o, -p) are mutually exclusive." << std::endl;
                return std::nullopt;
            }
            specifiedMode = OutputMode::DB_ONLY;
        }
        else if ((arg == "-y" || arg == "--year") && i + 1 < argc) {
            try {
                config.specified_year = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid year format provided." << std::endl;
                return std::nullopt;
            }
        }
        else if (arg == "-v" || arg == "--validate") {
            config.validate_only = true;
        }
        else if (arg[0] != '-' && config.log_filepath.empty()) {
            config.log_filepath = arg;
        }
        else {
            std::cerr << "Error: Unknown or invalid argument '" << arg << "'" << std::endl;
            printUsage(argv[0]);
            return std::nullopt;
        }
    }

    if (specifiedMode.has_value()) {
        config.output_mode = specifiedMode.value();
    } else {
        if (!config.validate_only) {
             config.output_mode = OutputMode::ALL;
        }
    }

    if (config.log_filepath.empty()) {
        std::cerr << "Error: A file or directory path is required. Use --help for more information." << std::endl;
        return std::nullopt;
    }

    std::filesystem::path exe_path = argv[0];
    config.db_path = (exe_path.parent_path() / "workouts.sqlite3").string();
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
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") return 0;
        }
        return 1;
    }

    ActionHandler handler;
    bool success = handler.run(configOpt.value());

    return success ? 0 : 1;
}