// cli/commands/convert_command.hpp
#ifndef CLI_COMMANDS_CONVERT_COMMAND_HPP_
#define CLI_COMMANDS_CONVERT_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ConvertCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "data"; }

  auto GetGroupDescription() const -> std::string override {
    return "Validate, transform, and persist workout data.";
  }

  auto GetCommandName() const -> std::string override { return "convert"; }

  auto GetCommandDescription() const -> std::string override {
    return "Convert workout logs into processed JSON files.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) + " data convert <path>";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {std::string(program_name) + " data convert logs"};
  }

  auto Parse(const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    if (args.size() < 2) {
      std::cerr << "Error: 'convert' command requires a <path> argument."
                << std::endl;
      return false;
    }
    if (args.size() > 2) {
      std::cerr << "Error: 'convert' command accepts exactly one <path> argument."
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
