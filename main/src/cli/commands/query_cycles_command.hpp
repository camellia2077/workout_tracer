// cli/commands/query_cycles_command.hpp
#ifndef CLI_COMMANDS_QUERY_CYCLES_COMMAND_HPP_
#define CLI_COMMANDS_QUERY_CYCLES_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli::commands {

class QueryCyclesCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "cycles"; }

  auto GetCategory() const -> std::string override { return "Analysis & Query"; }

  auto GetDescription() const -> std::string override {
    return "Query all stored training cycles.";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    config.action_ = ActionType::QueryCycles;
    return true;
  }
};

} // namespace cli::commands

#endif // CLI_COMMANDS_QUERY_CYCLES_COMMAND_HPP_
