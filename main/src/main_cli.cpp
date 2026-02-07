// main_cli.cpp

#include "application/action_handler.hpp"
#include "cli/commands/convert_command.hpp"
#include "cli/commands/export_command.hpp"
#include "cli/commands/ingest_command.hpp"
#include "cli/commands/insert_command.hpp"
#include "cli/commands/list_exercises_command.hpp"
#include "cli/commands/query_cycles_command.hpp"
#include "cli/commands/query_pr_command.hpp"
#include "cli/commands/validate_command.hpp"
#include "cli/commands/volume_command.hpp"
#include "cli/framework/application.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

auto main(int argc, char** argv) -> int {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  cli::framework::Application app("workout_calculator");

  app.RegisterCommand(std::make_unique<cli::commands::ValidateCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::ConvertCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::InsertCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::ExportCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::IngestCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::ListExercisesCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::QueryCyclesCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::QueryPRCommand>());
  app.RegisterCommand(std::make_unique<cli::commands::VolumeCommand>());

  auto config_opt = app.Parse(argc, argv);
  if (!config_opt.has_value()) {
    return 1;
  }

  auto exit_code = ActionHandler::Run(config_opt.value());

  return static_cast<int>(exit_code);
}