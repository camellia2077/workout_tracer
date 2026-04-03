#include "cli/framework/core_request_mapper.hpp"

#include "common/c_json_helper.hpp"

namespace cli::framework {

namespace {
auto MapActionType(ActionType action) -> workout_core_command_t {
  switch (action) {
    case ActionType::Validate:
      return WORKOUT_CORE_COMMAND_VALIDATE;
    case ActionType::Convert:
      return WORKOUT_CORE_COMMAND_CONVERT;
    case ActionType::Insert:
      return WORKOUT_CORE_COMMAND_INSERT;
    case ActionType::Export:
      return WORKOUT_CORE_COMMAND_EXPORT;
    case ActionType::Ingest:
      return WORKOUT_CORE_COMMAND_INGEST;
    case ActionType::QueryPR:
      return WORKOUT_CORE_COMMAND_QUERY_PR;
    case ActionType::ListExercises:
      return WORKOUT_CORE_COMMAND_LIST_EXERCISES;
    case ActionType::QueryCycles:
      return WORKOUT_CORE_COMMAND_QUERY_CYCLES;
    case ActionType::QueryVolume:
      return WORKOUT_CORE_COMMAND_QUERY_VOLUME;
    default:
      return WORKOUT_CORE_COMMAND_UNKNOWN;
  }
}

auto BuildRequestJson(const AppConfig& config) -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON_AddStringToObject(root.get(), "log_path", config.log_filepath_.c_str());
  cJSON_AddStringToObject(root.get(), "mapping_path",
                          config.mapping_path_.c_str());
  cJSON_AddStringToObject(root.get(), "base_path", config.base_path_.c_str());
  cJSON_AddStringToObject(root.get(), "type_filter",
                          config.type_filter_.c_str());
  cJSON_AddStringToObject(root.get(), "cycle_id_filter",
                          config.cycle_id_filter_.c_str());
  cJSON_AddStringToObject(root.get(), "display_unit",
                          config.display_unit_.c_str());

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string json_text(json_chars);
  cJSON_free(json_chars);
  return json_text;
}
}  // namespace

auto BuildCoreRequestFromConfig(const AppConfig& config)
    -> CoreRequestEnvelope {
  CoreRequestEnvelope envelope;
  envelope.command_ = MapActionType(config.action_);
  envelope.request_json_ = BuildRequestJson(config);
  return envelope;
}

}  // namespace cli::framework
