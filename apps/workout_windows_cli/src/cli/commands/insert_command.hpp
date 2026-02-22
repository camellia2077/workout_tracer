// cli/commands/insert_command.hpp
#ifndef CLI_COMMANDS_INSERT_COMMAND_HPP_
#define CLI_COMMANDS_INSERT_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class InsertCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "insert"; }

  auto GetCategory() const -> std::string override { return "Storage & Output"; }

  auto GetDescription() const -> std::string override {
    return "Insert JSON files into the database.";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    if (args.size() < 2) {
      std::cerr << "Error: 'insert' command requires a <path> argument."
                << std::endl;
      return false;
    }
    config.action_ = ActionType::Insert;
    config.log_filepath_ = args[1];
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_INSERT_COMMAND_HPP_
