// cli/commands/export_command.hpp
#ifndef CLI_COMMANDS_EXPORT_COMMAND_HPP_
#define CLI_COMMANDS_EXPORT_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ExportCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "export"; }

  auto GetCategory() const -> std::string override { return "Storage & Output"; }

  auto GetDescription() const -> std::string override {
    return "Export all data from the database to Markdown files.";
  }

  auto Parse([[maybe_unused]] const std::vector<std::string>& args, AppConfig& config) -> bool override {
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
