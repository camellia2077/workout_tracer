#ifndef CLI_COMMANDS_DISPLAY_UNIT_OPTION_HPP_
#define CLI_COMMANDS_DISPLAY_UNIT_OPTION_HPP_

#include <iostream>
#include <string_view>
#include <vector>

#include "application/action_handler.hpp"
#include "domain/services/weight_unit_service.hpp"

namespace cli::commands {

inline auto ParseDisplayUnitArgument(const std::vector<std::string>& args,
                                     size_t& index, AppConfig& config,
                                     std::string_view command_name) -> bool {
  if (index + 1 >= args.size()) {
    std::cerr << "Error: '--unit' requires a value." << std::endl;
    return false;
  }

  const auto normalized =
      WeightUnitService::NormalizeDisplayUnit(args[++index]);
  if (!normalized.has_value()) {
    std::cerr << "Error: Invalid value '" << args[index]
              << "' for '--unit' in '" << command_name
              << "' command. Use one of: original, kg, lb." << std::endl;
    return false;
  }

  config.display_unit_ = normalized.value();
  return true;
}

}  // namespace cli::commands

#endif  // CLI_COMMANDS_DISPLAY_UNIT_OPTION_HPP_
