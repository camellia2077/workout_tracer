// cli/commands/list_exercises_command.hpp
#ifndef CLI_COMMANDS_LIST_EXERCISES_COMMAND_HPP_
#define CLI_COMMANDS_LIST_EXERCISES_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ListExercisesCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "list"; }

  auto GetCategory() const -> std::string override { return "Analysis & Query"; }

  auto GetDescription() const -> std::string override {
    return "List all exercises, optionally filtered by type.";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    config.action_ = ActionType::ListExercises;
    for (size_t i = 1; i < args.size(); ++i) {
      if ((args[i] == "--type" || args[i] == "-t") && i + 1 < args.size()) {
        config.type_filter_ = args[i + 1];
        i++;
      }
    }
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_LIST_EXERCISES_COMMAND_HPP_
