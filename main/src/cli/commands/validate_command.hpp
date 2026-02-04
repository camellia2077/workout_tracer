// cli/commands/validate_command.hpp
#ifndef CLI_COMMANDS_VALIDATE_COMMAND_HPP_
#define CLI_COMMANDS_VALIDATE_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ValidateCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "validate"; }

  auto GetCategory() const -> std::string override { return "Project Tools"; }

  auto GetDescription() const -> std::string override {
    return "Only validate the log file format.";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    if (args.size() < 2) {
      std::cerr << "Error: 'validate' command requires a <path> argument."
                << std::endl;
      return false;
    }
    config.action_ = ActionType::Validate;
    config.log_filepath_ = args[1];
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_VALIDATE_COMMAND_HPP_
