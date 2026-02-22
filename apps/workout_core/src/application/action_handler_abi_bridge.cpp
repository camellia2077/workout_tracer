#include "application/action_handler_abi_bridge.hpp"

#include <optional>
#include <string>

#include "application/action_handler.hpp"
#include "common/c_json_helper.hpp"
#include "core/abi/workout_core_abi.h"

namespace {

auto GetJsonStringOrEmpty(const cJSON* root, const char* key) -> std::string {
  if (root == nullptr || key == nullptr) {
    return "";
  }

  cJSON* item = cJSON_GetObjectItemCaseSensitive(const_cast<cJSON*>(root), key);
  if (cJSON_IsString(item) != 0 && item->valuestring != nullptr) {
    return item->valuestring;
  }

  return "";
}

auto TryMapCommand(workout_core_command_t command)
    -> std::optional<ActionType> {
  switch (command) {
    case WORKOUT_CORE_COMMAND_VALIDATE:
      return ActionType::Validate;
    case WORKOUT_CORE_COMMAND_CONVERT:
      return ActionType::Convert;
    case WORKOUT_CORE_COMMAND_INSERT:
      return ActionType::Insert;
    case WORKOUT_CORE_COMMAND_EXPORT:
      return ActionType::Export;
    case WORKOUT_CORE_COMMAND_INGEST:
      return ActionType::Ingest;
    case WORKOUT_CORE_COMMAND_QUERY_PR:
      return ActionType::QueryPR;
    case WORKOUT_CORE_COMMAND_LIST_EXERCISES:
      return ActionType::ListExercises;
    case WORKOUT_CORE_COMMAND_QUERY_CYCLES:
      return ActionType::QueryCycles;
    case WORKOUT_CORE_COMMAND_QUERY_VOLUME:
      return ActionType::QueryVolume;
    case WORKOUT_CORE_COMMAND_UNKNOWN:
    default:
      return std::nullopt;
  }
}

auto IsRequiredPathAction(ActionType action) -> bool {
  return action == ActionType::Validate || action == ActionType::Convert ||
         action == ActionType::Insert || action == ActionType::Ingest;
}

auto BuildConfigFromRequest(const workout_core_request_t& request)
    -> std::optional<AppConfig> {
  auto action_opt = TryMapCommand(request.command);
  if (!action_opt.has_value()) {
    return std::nullopt;
  }

  const char* request_json = request.request_json_utf8;
  if (request_json == nullptr || request_json[0] == '\0') {
    request_json = "{}";
  }

  CJsonPtr request_root = MakeCJson(cJSON_Parse(request_json));
  if (request_root == nullptr || cJSON_IsObject(request_root.get()) == 0) {
    return std::nullopt;
  }

  AppConfig config{
      .action_ = action_opt.value(),
      .log_filepath_ = GetJsonStringOrEmpty(request_root.get(), "log_path"),
      .mapping_path_ = GetJsonStringOrEmpty(request_root.get(), "mapping_path"),
      .base_path_ = GetJsonStringOrEmpty(request_root.get(), "base_path"),
      .type_filter_ = GetJsonStringOrEmpty(request_root.get(), "type_filter"),
      .cycle_id_filter_ =
          GetJsonStringOrEmpty(request_root.get(), "cycle_id_filter"),
  };

  if (IsRequiredPathAction(config.action_) && config.log_filepath_.empty()) {
    return std::nullopt;
  }

  if ((config.action_ == ActionType::Validate ||
       config.action_ == ActionType::Convert ||
       config.action_ == ActionType::Ingest) &&
      config.mapping_path_.empty()) {
    return std::nullopt;
  }

  if (config.action_ == ActionType::QueryVolume &&
      (config.type_filter_.empty() || config.cycle_id_filter_.empty())) {
    return std::nullopt;
  }

  return config;
}

auto MapExitCode(AppExitCode exit_code) -> workout_core_status_code_t {
  switch (exit_code) {
    case AppExitCode::kSuccess:
      return WORKOUT_CORE_STATUS_SUCCESS;
    case AppExitCode::kInvalidArgs:
      return WORKOUT_CORE_STATUS_INVALID_ARGS;
    case AppExitCode::kValidationError:
      return WORKOUT_CORE_STATUS_VALIDATION_ERROR;
    case AppExitCode::kFileNotFound:
      return WORKOUT_CORE_STATUS_FILE_NOT_FOUND;
    case AppExitCode::kDatabaseError:
      return WORKOUT_CORE_STATUS_DATABASE_ERROR;
    case AppExitCode::kProcessingError:
      return WORKOUT_CORE_STATUS_PROCESSING_ERROR;
    case AppExitCode::kUnknownError:
    default:
      return WORKOUT_CORE_STATUS_UNKNOWN_ERROR;
  }
}

auto BuildExitMessage(AppExitCode exit_code) -> const char* {
  switch (exit_code) {
    case AppExitCode::kSuccess:
      return "";
    case AppExitCode::kInvalidArgs:
      return "invalid arguments";
    case AppExitCode::kValidationError:
      return "validation error";
    case AppExitCode::kFileNotFound:
      return "file not found";
    case AppExitCode::kDatabaseError:
      return "database error";
    case AppExitCode::kProcessingError:
      return "processing error";
    case AppExitCode::kUnknownError:
    default:
      return "unknown error";
  }
}

}  // namespace

extern "C" auto ExecuteByActionHandler(const workout_core_request_t* request)
    -> workout_core_result_t {
  if (request == nullptr) {
    return workout_core_make_result(WORKOUT_CORE_STATUS_INVALID_ARGS,
                                    "request must not be null", "{}");
  }

  auto config_opt = BuildConfigFromRequest(*request);
  if (!config_opt.has_value()) {
    return workout_core_make_result(
        WORKOUT_CORE_STATUS_INVALID_ARGS,
        "invalid request: missing required fields for command", "{}");
  }

  auto exit_code = ActionHandler::Run(config_opt.value());
  return workout_core_make_result(MapExitCode(exit_code),
                                  BuildExitMessage(exit_code), "{}");
}

auto RegisterActionHandlerCoreDelegate() -> void {
  workout_core_set_execute_delegate(&ExecuteByActionHandler);
}
