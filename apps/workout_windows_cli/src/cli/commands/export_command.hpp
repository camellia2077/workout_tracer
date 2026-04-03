// cli/commands/export_command.hpp
#ifndef CLI_COMMANDS_EXPORT_COMMAND_HPP_
#define CLI_COMMANDS_EXPORT_COMMAND_HPP_

#include "cli/commands/display_unit_option.hpp"
#include "cli/framework/command.hpp"
#include <cctype>
#include <iostream>

namespace cli {
namespace commands {

namespace {
inline auto IsExportCycleYearMonthFormat(std::string_view value) -> bool {
  return value.size() == 7 &&
         std::isdigit(static_cast<unsigned char>(value[0])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[1])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[2])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[3])) != 0 &&
         value[4] == '-' &&
         std::isdigit(static_cast<unsigned char>(value[5])) != 0 &&
         std::isdigit(static_cast<unsigned char>(value[6])) != 0;
}
}  // namespace

class ExportCommand : public framework::Command {
public:
  auto GetGroupName() const -> std::string override { return "report"; }

  auto GetGroupDescription() const -> std::string override {
    return "Generate reports from stored workout data.";
  }

  auto GetCommandName() const -> std::string override { return "export"; }

  auto GetCommandDescription() const -> std::string override {
    return "Export Markdown reports from the database.";
  }

  auto GetUsage(std::string_view program_name) const -> std::string override {
    return std::string(program_name) +
           " report export [--cycle <YYYY-MM>] [--unit <original|kg|lb>]";
  }

  auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> override {
    return {
        std::string(program_name) + " report export",
        std::string(program_name) + " report export --cycle 2025-07",
        std::string(program_name) + " report export --unit lb",
    };
  }

  auto GetOptions() const -> HelpEntries override {
    return {
        {"--cycle <YYYY-MM>", "Export only one cycle (month)."},
        {"--unit <original|kg|lb>",
         "Display report weights in original, kg, or lb."},
    };
  }

  auto Parse(const std::vector<std::string>& args,
             AppConfig& config) const -> bool override {
    config.action_ = ActionType::Export;

    for (size_t i = 1; i < args.size(); ++i) {
      if (args[i] == "--cycle") {
        if (i + 1 >= args.size()) {
          std::cerr << "Error: '--cycle' requires a value." << std::endl;
          return false;
        }
        config.cycle_id_filter_ = args[++i];
      } else if (args[i] == "--unit") {
        if (!ParseDisplayUnitArgument(args, i, config, "export")) {
          return false;
        }
      } else {
        std::cerr << "Error: Unknown argument '" << args[i]
                  << "' for 'export' command." << std::endl;
        return false;
      }
    }
    if (!config.cycle_id_filter_.empty() &&
        !IsExportCycleYearMonthFormat(config.cycle_id_filter_)) {
      std::cerr << "Error: '--cycle' must be in YYYY-MM format." << std::endl;
      return false;
    }

    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_EXPORT_COMMAND_HPP_
