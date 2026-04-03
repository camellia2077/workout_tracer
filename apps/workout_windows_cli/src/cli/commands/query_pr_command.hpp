// cli/commands/query_pr_command.hpp
#ifndef CLI_COMMANDS_QUERY_PR_COMMAND_HPP_
#define CLI_COMMANDS_QUERY_PR_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include "cli/commands/display_unit_option.hpp"

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
    return std::string(program_name) +
           " query pr [--unit <original|kg|lb>]";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {
        std::string(program_name) + " query pr",
        std::string(program_name) + " query pr --unit lb",
    };
  }

  auto GetOptions() const -> HelpEntries override {
    return {
        {"--unit <original|kg|lb>",
         "Display PR weights in original, kg, or lb."},
    };
  }

  auto Parse(const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    config.action_ = ActionType::QueryPR;

    for (size_t i = 1; i < args.size(); ++i) {
      if (args[i] == "--unit") {
        if (!ParseDisplayUnitArgument(args, i, config, "pr")) {
          return false;
        }
      } else {
        std::cerr << "Error: Unknown argument '" << args[i]
                  << "' for 'pr' command." << std::endl;
        return false;
      }
    }

    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_QUERY_PR_COMMAND_HPP_
