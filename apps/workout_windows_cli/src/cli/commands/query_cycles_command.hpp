// cli/commands/query_cycles_command.hpp
#ifndef CLI_COMMANDS_QUERY_CYCLES_COMMAND_HPP_
#define CLI_COMMANDS_QUERY_CYCLES_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli::commands {

class QueryCyclesCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "query"; }

  auto GetGroupDescription() const -> std::string override {
    return "Query analytics and summaries from stored workout data.";
  }

  auto GetCommandName() const -> std::string override { return "cycles"; }

  auto GetCommandDescription() const -> std::string override {
    return "List all stored training cycles.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) + " query cycles";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {std::string(program_name) + " query cycles"};
  }

  auto Parse([[maybe_unused]] const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    if (args.size() > 1) {
      std::cerr
          << "Error: 'cycles' command does not take any additional arguments."
          << std::endl;
      return false;
    }
    config.action_ = ActionType::QueryCycles;
    return true;
  }
};

} // namespace cli::commands

#endif // CLI_COMMANDS_QUERY_CYCLES_COMMAND_HPP_
