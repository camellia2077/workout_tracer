// cli/framework/application.cpp
#include "cli/framework/application.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

#include "common/version.hpp"

namespace fs = std::filesystem;

namespace cli::framework {

Application::Application(std::string app_name)
    : app_name_(std::move(app_name)) {}

auto Application::RegisterCommand(std::unique_ptr<Command> command) -> void {
  const std::string group_name = command->GetGroupName();
  auto& group = command_groups_[group_name];
  if (group.description_.empty()) {
    group.description_ = command->GetGroupDescription();
  }
  group.commands_[command->GetCommandName()] = std::move(command);
}

auto Application::Parse(int argc, char** argv) const -> ParseResult {
  ParseResult result{};
  const std::string program_name = GetProgramName(argc > 0 ? argv[0] : nullptr);

  if (argc < 2) {
    std::cerr << "Error: Missing command group." << std::endl;
    std::cerr << "Use '" << program_name << " --help' for details."
              << std::endl;
    return result;
  }

  std::vector<std::string> args(argv + 1, argv + argc);
  const std::string& group_name = args[0];

  if (IsHelpToken(group_name)) {
    PrintRootHelp(program_name);
    result.exit_code_ = 0;
    return result;
  }
  if (group_name == "-v" || group_name == "--version") {
    PrintVersion();
    result.exit_code_ = 0;
    return result;
  }

  const auto group_it = command_groups_.find(group_name);
  if (group_it == command_groups_.end()) {
    std::cerr << "Error: Unknown command group '" << group_name << "'."
              << std::endl;
    std::cerr << "Use '" << program_name << " --help' for details."
              << std::endl;
    return result;
  }

  if (args.size() == 1) {
    std::cerr << "Error: Missing subcommand for group '" << group_name << "'."
              << std::endl;
    std::cerr << "Use '" << program_name << " " << group_name
              << " --help' for details." << std::endl;
    return result;
  }

  const std::string& command_name = args[1];
  if (IsHelpToken(command_name)) {
    PrintGroupHelp(program_name, group_name);
    result.exit_code_ = 0;
    return result;
  }

  const Command* command = FindCommand(group_name, command_name);
  if (command == nullptr) {
    std::cerr << "Error: Unknown subcommand '" << command_name
              << "' for group '" << group_name << "'." << std::endl;
    std::cerr << "Use '" << program_name << " " << group_name
              << " --help' for details." << std::endl;
    return result;
  }

  const bool command_help_requested =
      std::any_of(args.begin() + 2, args.end(),
                  [](const std::string& token) { return IsHelpToken(token); });
  if (command_help_requested) {
    PrintCommandHelp(program_name, *command);
    result.exit_code_ = 0;
    return result;
  }

  AppConfig config;
  ResolveConfigPaths(config, argv[0]);
  const std::vector<std::string> command_args(args.begin() + 1, args.end());

  if (!command->Parse(command_args, config)) {
    std::cerr << "Usage: " << command->GetUsage(program_name) << std::endl;
    std::cerr << "Use '" << program_name << " " << group_name << " "
              << command_name << " --help' for details." << std::endl;
    return result;
  }

  result.config_ = config;
  result.exit_code_ = 0;
  return result;
}

auto Application::PrintRootHelp(std::string_view program_name) const -> void {
  std::cerr << "Usage: " << program_name << " <group> <command> [options]"
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Process workout logs, query stored data, and export reports."
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Available command groups:" << std::endl;

  for (const auto& [group_name, group] : command_groups_) {
    const size_t padding_size = group_name.length() < kGroupPaddingLength
                                    ? kGroupPaddingLength - group_name.length()
                                    : 1;
    std::cerr << "  " << group_name << std::string(padding_size, ' ')
              << group.description_ << std::endl;
  }
  std::cerr << std::endl;
  std::cerr << "Use '" << program_name
            << " <group> --help' to list commands in a group." << std::endl;
  std::cerr << "Use '" << program_name
            << " <group> <command> --help' for detailed command help."
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr << "  -h, --help             Show this help message and exit."
            << std::endl;
  std::cerr << "  -v, --version          Show version information and exit."
            << std::endl;
}

auto Application::PrintGroupHelp(std::string_view program_name,
                                 std::string_view group_name) const -> void {
  const auto group_it = command_groups_.find(std::string(group_name));
  if (group_it == command_groups_.end()) {
    return;
  }

  std::cerr << "Usage: " << program_name << " " << group_name
            << " <command> [options]" << std::endl;
  std::cerr << std::endl;
  std::cerr << group_it->second.description_ << std::endl;
  std::cerr << std::endl;
  std::cerr << "Commands in '" << group_name << "':" << std::endl;

  for (const auto& [command_name, command] : group_it->second.commands_) {
    const size_t padding_size =
        command_name.length() < kCommandPaddingLength
            ? kCommandPaddingLength - command_name.length()
            : 1;
    std::cerr << "  " << command_name << std::string(padding_size, ' ')
              << command->GetCommandDescription() << std::endl;
  }

  std::cerr << std::endl;
  std::cerr << "Use '" << program_name << " " << group_name
            << " <command> --help' for details." << std::endl;
}

auto Application::PrintCommandHelp(std::string_view program_name,
                                   const Command& command) const -> void {
  std::cerr << "Usage: " << command.GetUsage(program_name) << std::endl;
  std::cerr << std::endl;
  std::cerr << command.GetCommandDescription() << std::endl;

  const auto options = command.GetOptions();
  if (!options.empty()) {
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    for (const auto& [option_name, description] : options) {
      const size_t padding_size =
          option_name.length() < kOptionPaddingLength
              ? kOptionPaddingLength - option_name.length()
              : 1;
      std::cerr << "  " << option_name << std::string(padding_size, ' ')
                << description << std::endl;
    }
  }

  const auto examples = command.GetExamples(program_name);
  if (!examples.empty()) {
    std::cerr << std::endl;
    std::cerr << "Examples:" << std::endl;
    for (const auto& example : examples) {
      std::cerr << "  " << example << std::endl;
    }
  }
}

auto Application::PrintVersion() -> void {
  std::cout << BuildInfo::PROJECT_NAME << " version " << BuildInfo::VERSION
            << " (Built: " << BuildInfo::BUILD_DATE << ")" << std::endl;
}

auto Application::FindCommand(std::string_view group_name,
                              std::string_view command_name) const
    -> const Command* {
  const auto group_it = command_groups_.find(std::string(group_name));
  if (group_it == command_groups_.end()) {
    return nullptr;
  }

  const auto command_it =
      group_it->second.commands_.find(std::string(command_name));
  if (command_it == group_it->second.commands_.end()) {
    return nullptr;
  }

  return command_it->second.get();
}

auto Application::GetProgramName(const char* executable_path) const
    -> std::string {
  if (executable_path == nullptr) {
    return app_name_;
  }

  const fs::path executable(executable_path);
  const std::string filename = executable.filename().string();
  if (!filename.empty()) {
    return filename;
  }

  return app_name_;
}

auto Application::IsHelpToken(std::string_view token) -> bool {
  return token == "-h" || token == "--help";
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
