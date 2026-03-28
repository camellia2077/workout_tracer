// cli/commands/export_command.hpp
#ifndef CLI_COMMANDS_EXPORT_COMMAND_HPP_
#define CLI_COMMANDS_EXPORT_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ExportCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "report"; }

  auto GetGroupDescription() const -> std::string override {
    return "Generate reports from stored workout data.";
  }

  auto GetCommandName() const -> std::string override { return "export"; }

  auto GetCommandDescription() const -> std::string override {
    return "Export Markdown reports from the database.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) + " report export";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {std::string(program_name) + " report export"};
  }

  auto Parse([[maybe_unused]] const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    if (args.size() > 1) {
      std::cerr
          << "Error: 'export' command does not take any additional arguments."
          << std::endl;
      return false;
    }
    config.action_ = ActionType::Export;
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_EXPORT_COMMAND_HPP_
