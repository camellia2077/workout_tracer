// cli/framework/application.hpp
#ifndef CLI_FRAMEWORK_APPLICATION_HPP_
#define CLI_FRAMEWORK_APPLICATION_HPP_

#include "cli/framework/command.hpp"
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace cli {
namespace framework {

class Application {
public:
  Application(std::string app_name);
  
  auto RegisterCommand(std::unique_ptr<Command> command) -> void;
  
  auto Parse(int argc, char** argv) -> std::optional<AppConfig>;

private:
  std::string app_name_;
  std::map<std::string, std::unique_ptr<Command>> commands_;

  auto PrintUsage() const -> void;
  static auto PrintVersion() -> void;
  static auto ResolveConfigPaths(AppConfig& config, const char* executable_path) -> void;
  static constexpr int kPaddingLength = 20;
};

} // namespace framework
} // namespace cli

#endif // CLI_FRAMEWORK_APPLICATION_HPP_
