// cli/framework/command.hpp
#ifndef CLI_FRAMEWORK_COMMAND_HPP_
#define CLI_FRAMEWORK_COMMAND_HPP_

#include "application/action_handler.hpp" // For AppConfig

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cli {
namespace framework {

class Command {
public:
  using HelpEntries = std::vector<std::pair<std::string, std::string>>;

  virtual ~Command() = default;

  [[nodiscard]] virtual auto GetGroupName() const -> std::string = 0;

  [[nodiscard]] virtual auto GetGroupDescription() const -> std::string = 0;

  [[nodiscard]] virtual auto GetCommandName() const -> std::string = 0;

  [[nodiscard]] virtual auto GetCommandDescription() const -> std::string = 0;

  [[nodiscard]] virtual auto GetUsage(std::string_view program_name) const
      -> std::string = 0;

  [[nodiscard]] virtual auto GetExamples(std::string_view program_name) const
      -> std::vector<std::string> {
    return {};
  }

  [[nodiscard]] virtual auto GetOptions() const -> HelpEntries { return {}; }

  virtual auto Parse(const std::vector<std::string>& args,
                     AppConfig& config) const -> bool = 0;
};

} // namespace framework
} // namespace cli

#endif // CLI_FRAMEWORK_COMMAND_HPP_
