// cli/commands/ingest_command.hpp
#ifndef CLI_COMMANDS_INGEST_COMMAND_HPP_
#define CLI_COMMANDS_INGEST_COMMAND_HPP_

#include "cli/framework/command.hpp"
#include <iostream>

namespace cli {
namespace commands {

class IngestCommand : public framework::Command {
public:
  auto GetName() const -> std::string override { return "ingest"; }

  auto GetCategory() const -> std::string override { return "Storage & Output"; }

  auto GetDescription() const -> std::string override {
    return "Read a log file, validate/convert it, and insert directly to DB (skips JSON).";
  }

  auto Parse(const std::vector<std::string>& args, AppConfig& config) -> bool override {
    if (args.size() < 2) {
      std::cerr << "Error: 'ingest' command requires a <path> argument."
                << std::endl;
      return false;
    }
    config.action_ = ActionType::Ingest;
    config.log_filepath_ = args[1];
    return true;
  }
};

} // namespace commands
} // namespace cli

#endif // CLI_COMMANDS_INGEST_COMMAND_HPP_
