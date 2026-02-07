// cli/commands/query_pr_command.hpp
#ifndef CLI_COMMANDS_QUERY_PR_COMMAND_HPP_
#define CLI_COMMANDS_QUERY_PR_COMMAND_HPP_

#include "cli/framework/command.hpp"

namespace cli {
namespace commands {

class QueryPRCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "pr"; }

  auto GetCategory() const -> std::string override { return "Analysis & Query"; }

  auto GetDescription() const -> std::string override {
    return "Query historical Personal Records (PRs).";
  }

  auto Parse([[maybe_unused]] const std::vector<std::string>& args, AppConfig& config) -> bool override {
    config.action_ = ActionType::QueryPR;
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_QUERY_PR_COMMAND_HPP_
