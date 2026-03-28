// cli/framework/application.hpp
#ifndef CLI_FRAMEWORK_APPLICATION_HPP_
#define CLI_FRAMEWORK_APPLICATION_HPP_

#include "cli/framework/command.hpp"
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace cli {
namespace framework {

struct ParseResult {
  std::optional<AppConfig> config_;
  int exit_code_ = 1;
};

class Application {
public:
  Application(std::string app_name);
  
  auto RegisterCommand(std::unique_ptr<Command> command) -> void;
  
  auto Parse(int argc, char** argv) const -> ParseResult;

private:
  struct CommandGroup {
    std::string description_;
    std::map<std::string, std::unique_ptr<Command>> commands_;
  };

  std::string app_name_;
  std::map<std::string, CommandGroup> command_groups_;

  auto PrintRootHelp(std::string_view program_name) const -> void;
  auto PrintGroupHelp(std::string_view program_name,
                      std::string_view group_name) const -> void;
  auto PrintCommandHelp(std::string_view program_name,
                        const Command& command) const -> void;
  [[nodiscard]] auto FindCommand(std::string_view group_name,
                                 std::string_view command_name) const
      -> const Command*;
  [[nodiscard]] auto GetProgramName(const char* executable_path) const
      -> std::string;
  static auto IsHelpToken(std::string_view token) -> bool;
  static auto PrintVersion() -> void;
  static auto ResolveConfigPaths(AppConfig& config, const char* executable_path) -> void;
  static constexpr int kGroupPaddingLength = 10;
  static constexpr int kCommandPaddingLength = 12;
  static constexpr int kOptionPaddingLength = 18;
};

} // namespace framework
} // namespace cli

#endif // CLI_FRAMEWORK_APPLICATION_HPP_
