// cli/commands/convert_command.hpp
#ifndef CLI_COMMANDS_CONVERT_COMMAND_HPP_
#define CLI_COMMANDS_CONVERT_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ConvertCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "convert"; }

  auto GetCategory() const -> std::string override { return "Project Tools"; }

  auto GetDescription() const -> std::string override {
    return "Convert the log file to JSON format.";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    if (args.size() < 2) {
      std::cerr << "Error: 'convert' command requires a <path> argument."
                << std::endl;
      return false;
    }
    config.action_ = ActionType::Convert;
    config.log_filepath_ = args[1];
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_CONVERT_COMMAND_HPP_
