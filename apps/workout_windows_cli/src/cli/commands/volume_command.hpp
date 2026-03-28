// cli/commands/volume_command.hpp
#ifndef CLI_COMMANDS_VOLUME_COMMAND_HPP_
#define CLI_COMMANDS_VOLUME_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli::commands {

class VolumeCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "query"; }

  auto GetGroupDescription() const -> std::string override {
    return "Query analytics and summaries from stored workout data.";
  }

  auto GetCommandName() const -> std::string override { return "volume"; }

  auto GetCommandDescription() const -> std::string override {
    return "Show total and average daily volume for a cycle and type.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) +
           " query volume --type <type> --cycle <cycle>";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {
        std::string(program_name) +
        " query volume --type push --cycle 2025-07-05",
    };
  }

  auto GetOptions() const -> HelpEntries override {
    return {
        {"--type <type>", "Workout type to query."},
        {"--cycle <cycle>", "Training cycle identifier to query."},
    };
  }

  auto Parse(const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    config.action_ = ActionType::QueryVolume;

    for (size_t i = 1; i < args.size(); ++i) {
      if (args[i] == "--type") {
        if (i + 1 >= args.size()) {
          std::cerr << "Error: '--type' requires a value." << std::endl;
          return false;
        }
        config.type_filter_ = args[++i];
      } else if (args[i] == "--cycle") {
        if (i + 1 >= args.size()) {
          std::cerr << "Error: '--cycle' requires a value." << std::endl;
          return false;
        }
        config.cycle_id_filter_ = args[++i];
      } else {
        std::cerr << "Error: Unknown argument '" << args[i]
                  << "' for 'volume' command." << std::endl;
        return false;
      }
    }

    if (config.type_filter_.empty() || config.cycle_id_filter_.empty()) {
      std::cerr
          << "Error: Both --type and --cycle are required for volume query."
          << std::endl;
      return false;
    }

    return true;
  }
};

} // namespace cli::commands

#endif // CLI_COMMANDS_VOLUME_COMMAND_HPP_
