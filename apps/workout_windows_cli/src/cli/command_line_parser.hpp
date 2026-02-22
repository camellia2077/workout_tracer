#ifndef CLI_COMMAND_LINE_PARSER_HPP_
#define CLI_COMMAND_LINE_PARSER_HPP_

#include "application/action_handler.hpp"
#include <optional>

class CommandLineParser {
public:
  // Parse command line arguments and return configuration
  [[nodiscard]] static auto Parse(int argc, char** argv) -> std::optional<AppConfig>;

private:
  // Helper methods
  static auto ResolveConfigPaths(AppConfig& config, const char* executable_path) -> void;
  static auto PrintVersion() -> void;
  static auto PrintUsage(const char* program_name) -> void;
};

#endif // CLI_COMMAND_LINE_PARSER_HPP_
