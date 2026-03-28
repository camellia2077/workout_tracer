// cli/commands/list_exercises_command.hpp
#ifndef CLI_COMMANDS_LIST_EXERCISES_COMMAND_HPP_
#define CLI_COMMANDS_LIST_EXERCISES_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class ListExercisesCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "query"; }

  auto GetGroupDescription() const -> std::string override {
    return "Query analytics and summaries from stored workout data.";
  }

  auto GetCommandName() const -> std::string override { return "list"; }

  auto GetCommandDescription() const -> std::string override {
    return "List exercises, optionally filtered by workout type.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) + " query list [--type <type>]";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {
        std::string(program_name) + " query list",
        std::string(program_name) + " query list --type push",
    };
  }

  auto GetOptions() const -> HelpEntries override {
    return {
        {"--type <type>", "Filter exercises by workout type."},
        {"-t <type>", "Alias for --type."},
    };
  }

  auto Parse(const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    config.action_ = ActionType::ListExercises;
    for (size_t i = 1; i < args.size(); ++i) {
      if (args[i] == "--type" || args[i] == "-t") {
        if (i + 1 >= args.size()) {
          std::cerr << "Error: '--type' requires a value." << std::endl;
          return false;
        }
        config.type_filter_ = args[++i];
        continue;
      }
      std::cerr << "Error: Unknown argument '" << args[i]
                << "' for 'list' command." << std::endl;
      return false;
    }
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_LIST_EXERCISES_COMMAND_HPP_
