// application/action_handler.cpp

#include "application/action_handler.hpp"

#include <filesystem>

#include "application/database_handler.hpp"
#include "application/file_processor_handler.hpp"
#include "infrastructure/config/file_mapping_provider.hpp"
#include "infrastructure/converter/log_parser.hpp"
#include "infrastructure/persistence/repository/sqlite_workout_repository.hpp"
#include "infrastructure/reporting/exporter/sqlite_report_exporter.hpp"

namespace fs = std::filesystem;

namespace {

auto BuildDatabasePath(const AppConfig& config) -> std::string {
  return (fs::path(config.base_path_) / "output" / "db" /
          "workout_logs.sqlite3")
      .string();
}

auto RunFileAction(const AppConfig& config) -> AppExitCode {
  LogParser parser;
  FileMappingProvider mapping_provider;
  FileProcessorHandler file_processor(parser, mapping_provider);
  return file_processor.Handle(config);
}

auto RunDatabaseAction(const AppConfig& config) -> AppExitCode {
  SqliteWorkoutRepository repository(BuildDatabasePath(config));
  SqliteReportExporter report_exporter(BuildDatabasePath(config));
  DatabaseHandler database_handler(repository, report_exporter);
  return database_handler.Handle(config);
}

auto RunIngestAction(const AppConfig& config) -> AppExitCode {
  LogParser parser;
  FileMappingProvider mapping_provider;
  FileProcessorHandler file_processor(parser, mapping_provider);

  auto data_opt =
      file_processor.ProcessFile({.file_path_ = config.log_filepath_,
                                  .mapping_path_ = config.mapping_path_});
  if (!data_opt.has_value()) {
    return AppExitCode::kProcessingError;
  }

  SqliteWorkoutRepository repository(BuildDatabasePath(config));
  SqliteReportExporter report_exporter(BuildDatabasePath(config));
  DatabaseHandler database_handler(repository, report_exporter);
  return database_handler.InsertData(data_opt.value(), config);
}

}  // namespace

auto ActionHandler::Run(const AppConfig& config) -> AppExitCode {
  if (config.action_ == ActionType::Validate ||
      config.action_ == ActionType::Convert) {
    return RunFileAction(config);
  }

  if (config.action_ == ActionType::Insert ||
      config.action_ == ActionType::Export ||
      config.action_ == ActionType::QueryPR ||
      config.action_ == ActionType::ListExercises ||
      config.action_ == ActionType::QueryCycles ||
      config.action_ == ActionType::QueryVolume) {
    return RunDatabaseAction(config);
  }

  if (config.action_ == ActionType::Ingest) {
    return RunIngestAction(config);
  }

  return AppExitCode::kUnknownError;
}
