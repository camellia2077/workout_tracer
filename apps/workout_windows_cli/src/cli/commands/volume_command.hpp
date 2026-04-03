// cli/commands/volume_command.hpp
#ifndef CLI_COMMANDS_VOLUME_COMMAND_HPP_
#define CLI_COMMANDS_VOLUME_COMMAND_HPP_

#include "cli/commands/display_unit_option.hpp"
#include "cli/framework/command.hpp"
#include <cctype>
#include <iostream>

namespace cli::commands {

namespace {
inline auto IsYearMonthFormat(std::string_view value) -> bool {
  return value.size() == 7 &&
         std::isdigit(static_cast<unsigned char>(value[0])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[1])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[2])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[3])) != 0 &&
         value[4] == '-' &&
         std::isdigit(static_cast<unsigned char>(value[5])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[6])) != 0;
}
} // namespace

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
           " query volume --type <type> --cycle <cycle> "
           "[--unit <original|kg|lb>]";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {
        std::string(program_name) +
        " query volume --type push --cycle 2025-07",
        std::string(program_name) +
        " query volume --type push --cycle 2025-07 --unit lb",
    };
  }

  auto GetOptions() const -> HelpEntries override {
    return {
        {"--type <type>", "Workout type to query."},
        {"--cycle <cycle>", "Training cycle identifier (YYYY-MM)."},
        {"--unit <original|kg|lb>",
         "Display aggregate weights in original, kg, or lb."},
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
      } else if (args[i] == "--unit") {
        if (!ParseDisplayUnitArgument(args, i, config, "volume")) {
          return false;
        }
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
    if (!IsYearMonthFormat(config.cycle_id_filter_)) {
      std::cerr << "Error: '--cycle' must be in YYYY-MM format." << std::endl;
      return false;
    }

    return true;
  }
};

} // namespace cli::commands

#endif // CLI_COMMANDS_VOLUME_COMMAND_HPP_
