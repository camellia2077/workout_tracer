// cli/commands/query_pr_command.hpp
#ifndef CLI_COMMANDS_QUERY_PR_COMMAND_HPP_
#define CLI_COMMANDS_QUERY_PR_COMMAND_HPP_

#include "cli/framework/command.hpp"

#include <iostream>

namespace cli {
namespace commands {

class QueryPRCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "query"; }

  auto GetGroupDescription() const -> std::string override {
    return "Query analytics and summaries from stored workout data.";
  }

  auto GetCommandName() const -> std::string override { return "pr"; }

  auto GetCommandDescription() const -> std::string override {
    return "Show historical personal records.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) + " query pr";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {std::string(program_name) + " query pr"};
  }

  auto Parse([[maybe_unused]] const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    if (args.size() > 1) {
      std::cerr << "Error: 'pr' command does not take any additional arguments."
                << std::endl;
      return false;
    }
    config.action_ = ActionType::QueryPR;
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_QUERY_PR_COMMAND_HPP_
