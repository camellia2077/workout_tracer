// application/action_handler.cpp

#include "application/action_handler.hpp"

#include "application/database_handler.hpp"
#include "application/file_processor_handler.hpp"
#include "infrastructure/config/file_mapping_provider.hpp"
#include "infrastructure/converter/log_parser.hpp"

auto ActionHandler::Run(const AppConfig& config) -> bool {
  if (config.action_ == ActionType::Validate ||
      config.action_ == ActionType::Convert) {
    LogParser parser;
    FileMappingProvider mapping_provider;
    FileProcessorHandler file_processor(parser, mapping_provider);
    return file_processor.Handle(config);
  }

  if (config.action_ == ActionType::Insert ||
      config.action_ == ActionType::Export ||
      config.action_ == ActionType::QueryPR ||
      config.action_ == ActionType::ListExercises ||
      config.action_ == ActionType::QueryCycles ||
      config.action_ == ActionType::QueryVolume) {
    return DatabaseHandler::Handle(config);
  }

  if (config.action_ == ActionType::Ingest) {
    LogParser parser;
    FileMappingProvider mapping_provider;
    FileProcessorHandler file_processor(parser, mapping_provider);

    auto data_opt =
        file_processor.ProcessFile({.file_path_ = config.log_filepath_,
                                    .mapping_path_ = config.mapping_path_});
    if (data_opt.has_value()) {
      return DatabaseHandler::InsertData(data_opt.value(), config);
    }
    return false;
  }

  return false;
}