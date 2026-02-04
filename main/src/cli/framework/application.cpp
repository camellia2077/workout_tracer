// cli/framework/application.cpp
#include "cli/framework/application.hpp"

#include <filesystem>
#include <iostream>
#include <utility>

#include "common/version.hpp"

namespace fs = std::filesystem;

namespace cli::framework {

Application::Application(std::string app_name)
    : app_name_(std::move(app_name)) {}

auto Application::RegisterCommand(std::unique_ptr<Command> command) -> void {
  commands_[command->GetName()] = std::move(command);
}

auto Application::Parse(int argc, char** argv) -> std::optional<AppConfig> {
  if (argc < 2) {
    PrintUsage();
    return std::nullopt;
  }

  std::vector<std::string> args(argv + 1, argv + argc);
  std::string cmd_name = args[0];

  if (cmd_name == "-h" || cmd_name == "--help") {
    PrintUsage();
    return std::nullopt;
  }
  if (cmd_name == "-v" || cmd_name == "--version") {
    PrintVersion();
    return std::nullopt;
  }

  auto cmd_it = commands_.find(cmd_name);
  if (cmd_it == commands_.end()) {
    std::cerr << "Error: Unknown command '" << cmd_name << "'" << std::endl;
    PrintUsage();
    return std::nullopt;
  }

  AppConfig config;
  ResolveConfigPaths(config, argv[0]);

  if (!cmd_it->second->Parse(args, config)) {
    return std::nullopt;
  }

  return config;
}

auto Application::PrintUsage() const -> void {
  std::cerr << "Usage: " << app_name_ << " <command> [<path>] [options]"
            << std::endl;
  std::cerr << std::endl;

  // Group commands by category
  std::map<std::string, std::vector<const Command*>> grouped_commands;
  for (const auto& [name, cmd] : commands_) {
    grouped_commands[cmd->GetCategory()].push_back(cmd.get());
  }

  for (const auto& [category, cmds] : grouped_commands) {
    std::cerr << category << ":" << std::endl;
    for (const auto* cmd : cmds) {
      std::string padding(kPaddingLength - cmd->GetName().length(), ' ');
      std::cerr << "  " << cmd->GetName() << padding << cmd->GetDescription()
                << std::endl;
    }
    std::cerr << std::endl;
  }
  std::cerr << "Options:" << std::endl;
  std::cerr << "  -h, --help             Show this help message and exit."
            << std::endl;
  std::cerr << "  -v, --version          Show version information and exit."
            << std::endl;
}

auto Application::PrintVersion() -> void {
  std::cout << BuildInfo::PROJECT_NAME << " version " << BuildInfo::VERSION
            << " (Built: " << BuildInfo::BUILD_DATE << ")" << std::endl;
}

auto Application::ResolveConfigPaths(AppConfig& config,
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

}  // namespace cli::framework
