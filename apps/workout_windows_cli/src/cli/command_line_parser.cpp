// cli/command_line_parser.cpp

#include "cli/command_line_parser.hpp"
#include "common/version.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

auto CommandLineParser::Parse(int argc, char** argv) -> std::optional<AppConfig> {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return std::nullopt;
  }

  std::vector<std::string> args(argv + 1, argv + argc);
  std::string command = args[0];

  if (command == "-h" || command == "--help") {
    PrintUsage(argv[0]);
    return std::nullopt;
  }
  if (command == "-v" || command == "--version") {
    PrintVersion();
    return std::nullopt;
  }

  AppConfig config;

  if (command == "export") {
    if (args.size() > 1) {
      std::cerr
          << "Error: 'export' command does not take any additional arguments."
          << std::endl;
      return std::nullopt;
    }
    config.action_ = ActionType::Export;
  } else if (command == "validate" || command == "convert" ||
             command == "insert") {
    if (args.size() < 2) {
      std::cerr << "Error: '" << command
                << "' command requires a <path> argument." << std::endl;
      return std::nullopt;
    }

    config.log_filepath_ = args[1];

    if (command == "validate") {
      config.action_ = ActionType::Validate;
    }
    if (command == "convert") {
      config.action_ = ActionType::Convert;
    }
    if (command == "insert") {
      config.action_ = ActionType::Insert;
    }

    if (args.size() > 2) {
      std::cerr << "Error: Unknown or invalid argument '" << args[2] << "'"
                << std::endl;
      return std::nullopt;
    }
  } else {
    std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
    PrintUsage(argv[0]);
    return std::nullopt;
  }

  ResolveConfigPaths(config, argv[0]);

  return config;
}

auto CommandLineParser::ResolveConfigPaths(AppConfig& config,
                                            const char* executable_path) -> void {
  try {
    if (executable_path == nullptr) {
      return;
    }
    
    fs::path exe_path = fs::absolute(fs::path(executable_path));
    fs::path exe_dir = exe_path.parent_path();
    fs::path config_dir = exe_dir / "config";
    fs::path mapping_file = config_dir / "mapping.json";

    config.base_path_ = exe_dir.string();
    config.mapping_path_ = mapping_file.string();

    if (!fs::exists(config.mapping_path_)) {
      std::cerr << "Warning: Configuration file not found at: "
                << config.mapping_path_ << std::endl;
    }
  } catch (const fs::filesystem_error& e) {
    std::cerr << "Error resolving paths: " << e.what() << std::endl;
  }
}

auto CommandLineParser::PrintVersion() -> void {
  std::cout << BuildInfo::PROJECT_NAME << " version " << BuildInfo::VERSION
            << " (Built: " << BuildInfo::BUILD_DATE << ")" << std::endl;
}

auto CommandLineParser::PrintUsage(const char* program_name) -> void {
  if (program_name == nullptr) {
    program_name = "workout_tracker";
  }
  
  std::cerr << "Usage: " << program_name << " <command> [<path>] [options]"
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Description:" << std::endl;
  std::cerr << "  Processes, validates, inserts, or exports workout logs."
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Commands:" << std::endl;
  std::cerr << "  validate <path>        Only validate the log file format."
            << std::endl;
  std::cerr << "  convert <path>         Convert the log file to JSON format."
            << std::endl;
  std::cerr << "  insert <path>          Insert JSON files into the database."
            << std::endl;
  std::cerr << "  export                 Export all data from the database to Markdown files."
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr << "  -h, --help             Show this help message and exit."
            << std::endl;
  std::cerr << "  -v, --version          Show version information and exit."
            << std::endl;
}