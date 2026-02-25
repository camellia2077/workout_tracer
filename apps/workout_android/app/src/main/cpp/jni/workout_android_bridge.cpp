#include <jni.h>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "application/action_handler.hpp"
#include "application/exit_code.hpp"
#include "common/c_json_helper.hpp"
#include "common/version.hpp"
#include "core/abi/workout_core_abi.h"
#include "infrastructure/persistence/facade/query_facade.hpp"
#include "infrastructure/persistence/manager/db_manager.hpp"

namespace fs = std::filesystem;

namespace {

struct BridgeResult {
  workout_core_status_code_t status_code =
      WORKOUT_CORE_STATUS_UNKNOWN_ERROR;
  std::string message;
  std::string payload_json = "{}";
};

auto FromJString(JNIEnv* env, jstring value) -> std::string {
  if (env == nullptr || value == nullptr) {
    return "";
  }

  const char* chars = env->GetStringUTFChars(value, nullptr);
  if (chars == nullptr) {
    return "";
  }

  std::string text(chars);
  env->ReleaseStringUTFChars(value, chars);
  return text;
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

auto MakeBridgeResult(AppExitCode exit_code,
                      const std::string& payload_json = "{}") -> BridgeResult {
  return BridgeResult{
      .status_code = MapExitCode(exit_code),
      .message = BuildExitMessage(exit_code),
      .payload_json = payload_json,
  };
}

auto MakeNativeResultObject(JNIEnv* env, const BridgeResult& result) -> jobject {
  if (env == nullptr) {
    return nullptr;
  }

  jclass clazz = env->FindClass("com/workout/calculator/core/NativeResult");
  if (clazz == nullptr) {
    return nullptr;
  }

  jmethodID ctor = env->GetMethodID(
      clazz, "<init>", "(ILjava/lang/String;Ljava/lang/String;)V");
  if (ctor == nullptr) {
    env->DeleteLocalRef(clazz);
    return nullptr;
  }

  jstring message = env->NewStringUTF(result.message.c_str());
  jstring payload = env->NewStringUTF(result.payload_json.c_str());
  jobject native_result = env->NewObject(
      clazz, ctor, static_cast<jint>(result.status_code), message, payload);

  env->DeleteLocalRef(message);
  env->DeleteLocalRef(payload);
  env->DeleteLocalRef(clazz);
  return native_result;
}

auto BuildDbPath(const std::string& base_path) -> fs::path {
  return fs::path(base_path) / "output" / "db" / "workout_logs.sqlite3";
}

auto BuildExercisePayload(const std::vector<ExerciseInfo>& exercises)
    -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON* array = cJSON_AddArrayToObject(root.get(), "exercises");
  for (const auto& exercise : exercises) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "name", exercise.name.c_str());
    cJSON_AddStringToObject(item, "type", exercise.type.c_str());
    cJSON_AddItemToArray(array, item);
  }

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto BuildPrPayload(const std::vector<PersonalRecord>& prs) -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON* array = cJSON_AddArrayToObject(root.get(), "prs");
  for (const auto& pr : prs) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "exercise_name", pr.exercise_name.c_str());
    cJSON_AddNumberToObject(item, "max_weight", pr.max_weight);
    cJSON_AddNumberToObject(item, "reps", pr.reps);
    cJSON_AddStringToObject(item, "date", pr.date.c_str());
    cJSON_AddNumberToObject(item, "estimated_1rm_epley", pr.estimated_1rm_epley);
    cJSON_AddNumberToObject(item, "estimated_1rm_brzycki",
                            pr.estimated_1rm_brzycki);
    cJSON_AddItemToArray(array, item);
  }

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto QueryExercises(const std::string& base_path,
                    const std::string& type_filter) -> BridgeResult {
  const fs::path db_path = BuildDbPath(base_path);
  if (!fs::exists(db_path)) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_FILE_NOT_FOUND,
        .message = "database file not found",
        .payload_json = "{}",
    };
  }

  DbManager db_manager(db_path.string());
  if (!db_manager.Open()) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_DATABASE_ERROR,
        .message = "failed to open database",
        .payload_json = "{}",
    };
  }

  auto exercises =
      QueryFacade::GetExercisesByType(db_manager.GetConnection(), type_filter);
  return BridgeResult{
      .status_code = WORKOUT_CORE_STATUS_SUCCESS,
      .message = "",
      .payload_json = BuildExercisePayload(exercises),
  };
}

auto QueryPrs(const std::string& base_path) -> BridgeResult {
  const fs::path db_path = BuildDbPath(base_path);
  if (!fs::exists(db_path)) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_FILE_NOT_FOUND,
        .message = "database file not found",
        .payload_json = "{}",
    };
  }

  DbManager db_manager(db_path.string());
  if (!db_manager.Open()) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_DATABASE_ERROR,
        .message = "failed to open database",
        .payload_json = "{}",
    };
  }

  auto prs = QueryFacade::QueryAllPRs(db_manager.GetConnection());
  return BridgeResult{
      .status_code = WORKOUT_CORE_STATUS_SUCCESS,
      .message = "",
      .payload_json = BuildPrPayload(prs),
  };
}

}  // namespace

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_ingest(
    JNIEnv* env, jobject /*thiz*/, jstring log_path, jstring mapping_path,
    jstring base_path) {
  const std::string log_path_text = FromJString(env, log_path);
  const std::string mapping_path_text = FromJString(env, mapping_path);
  const std::string base_path_text = FromJString(env, base_path);

  if (log_path_text.empty() || mapping_path_text.empty() ||
      base_path_text.empty()) {
    return MakeNativeResultObject(
        env,
        BridgeResult{
            .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
            .message = "log_path, mapping_path and base_path are required",
            .payload_json = "{}",
        });
  }

  AppConfig config{
      .action_ = ActionType::Ingest,
      .log_filepath_ = log_path_text,
      .mapping_path_ = mapping_path_text,
      .base_path_ = base_path_text,
      .type_filter_ = "",
      .cycle_id_filter_ = "",
  };

  AppExitCode exit_code = ActionHandler::Run(config);
  return MakeNativeResultObject(env, MakeBridgeResult(exit_code));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_listExercises(
    JNIEnv* env, jobject /*thiz*/, jstring base_path, jstring type_filter) {
  const std::string base_path_text = FromJString(env, base_path);
  const std::string type_filter_text = FromJString(env, type_filter);

  if (base_path_text.empty()) {
    return MakeNativeResultObject(
        env, BridgeResult{
                 .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
                 .message = "base_path is required",
                 .payload_json = "{}",
             });
  }

  return MakeNativeResultObject(
      env, QueryExercises(base_path_text, type_filter_text));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_queryPrs(
    JNIEnv* env, jobject /*thiz*/, jstring base_path) {
  const std::string base_path_text = FromJString(env, base_path);

  if (base_path_text.empty()) {
    return MakeNativeResultObject(
        env, BridgeResult{
                 .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
                 .message = "base_path is required",
                 .payload_json = "{}",
             });
  }

  return MakeNativeResultObject(env, QueryPrs(base_path_text));
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_getCoreVersion(
    JNIEnv* env, jobject /*thiz*/) {
  if (env == nullptr) {
    return nullptr;
  }

  const std::string version_text(BuildInfo::VERSION);
  return env->NewStringUTF(version_text.c_str());
}
