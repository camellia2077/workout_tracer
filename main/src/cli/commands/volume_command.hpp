// cli/commands/volume_command.hpp
#ifndef CLI_COMMANDS_VOLUME_COMMAND_HPP_
#define CLI_COMMANDS_VOLUME_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli::commands {

class VolumeCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "volume"; }

  auto GetCategory() const -> std::string override { return "Analysis & Query"; }

  auto GetDescription() const -> std::string override {
    return "Query total and average daily volume for a specific cycle and type.";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    config.action_ = ActionType::QueryVolume;
    
    for (size_t i = 1; i < args.size(); ++i) {
      if (args[i] == "--type" && i + 1 < args.size()) {
        config.type_filter_ = args[++i];
      } else if (args[i] == "--cycle" && i + 1 < args.size()) {
        config.cycle_id_filter_ = args[++i];
      }
    }

    if (config.type_filter_.empty() || config.cycle_id_filter_.empty()) {
      std::cerr << "Error: Both --type and --cycle are required for volume query." << std::endl;
      return false;
    }

    return true;
  }
};

} // namespace cli::commands

#endif // CLI_COMMANDS_VOLUME_COMMAND_HPP_
