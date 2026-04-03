#include <jni.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "application/action_handler.hpp"
#include "application/file_processor_handler.hpp"
#include "application/interfaces/archive_models.hpp"
#include "application/exit_code.hpp"
#include "common/c_json_helper.hpp"
#include "common/version.hpp"
#include "core/application/core_error_code.hpp"
#include "core/abi/workout_core_abi.h"
#include "infrastructure/archive/zip_archive_service.hpp"
#include "infrastructure/config/file_mapping_provider.hpp"
#include "infrastructure/converter/log_parser.hpp"
#include "infrastructure/persistence/facade/query_facade.hpp"
#include "infrastructure/persistence/manager/db_manager.hpp"
#include "infrastructure/persistence/repository/sqlite_workout_repository.hpp"
#include "infrastructure/reporting/facade/report_facade.hpp"

namespace fs = std::filesystem;

namespace {

struct BridgeResult {
  workout_core_status_code_t status_code =
      WORKOUT_CORE_STATUS_UNKNOWN_ERROR;
  std::string message;
  std::string payload_json = "{}";
};

struct ImportRebuildSummary {
  struct FailureDetail {
    std::string file;
    std::string stage;
    std::vector<std::string> diagnostics;
  };

  int processed_txt_count = 0;
  int imported_txt_count = 0;
  std::vector<std::string> failed_files;
  std::vector<FailureDetail> failed_details;
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

auto MapCoreErrorCode(CoreErrorCode error_code) -> workout_core_status_code_t {
  switch (error_code) {
    case CoreErrorCode::kSuccess:
      return WORKOUT_CORE_STATUS_SUCCESS;
    case CoreErrorCode::kInvalidArgs:
      return WORKOUT_CORE_STATUS_INVALID_ARGS;
    case CoreErrorCode::kValidationError:
      return WORKOUT_CORE_STATUS_VALIDATION_ERROR;
    case CoreErrorCode::kFileNotFound:
      return WORKOUT_CORE_STATUS_FILE_NOT_FOUND;
    case CoreErrorCode::kDatabaseError:
      return WORKOUT_CORE_STATUS_DATABASE_ERROR;
    case CoreErrorCode::kProcessingError:
      return WORKOUT_CORE_STATUS_PROCESSING_ERROR;
    case CoreErrorCode::kNotImplemented:
    case CoreErrorCode::kUnknownError:
    default:
      return WORKOUT_CORE_STATUS_UNKNOWN_ERROR;
  }
}

auto BuildStagingDbPath(const std::string& base_path) -> fs::path {
  return fs::path(base_path) / "output" / "db" / "workout_logs.staging.sqlite3";
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
    cJSON_AddNumberToObject(item, "max_weight_kg", pr.max_weight);
    cJSON_AddStringToObject(item, "original_unit", pr.original_unit.c_str());
    cJSON_AddNumberToObject(item, "original_weight_value",
                            pr.original_weight_value);
    cJSON_AddNumberToObject(item, "reps", pr.reps);
    cJSON_AddStringToObject(item, "date", pr.date.c_str());
    cJSON_AddNumberToObject(item, "estimated_1rm_epley_kg",
                            pr.estimated_1rm_epley);
    cJSON_AddNumberToObject(item, "estimated_1rm_brzycki_kg",
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

auto BuildCyclesPayload(const std::vector<CycleRecord>& cycles) -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON* array = cJSON_AddArrayToObject(root.get(), "cycles");
  for (const auto& cycle : cycles) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "cycle_id", cycle.cycle_id.c_str());
    cJSON_AddNumberToObject(item, "total_days", cycle.total_days);
    cJSON_AddStringToObject(item, "type", cycle.type.c_str());
    cJSON_AddStringToObject(item, "start_date", cycle.start_date.c_str());
    cJSON_AddStringToObject(item, "end_date", cycle.end_date.c_str());
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

auto BuildCycleVolumesPayload(const std::vector<VolumeStats>& volumes)
    -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON* array = cJSON_AddArrayToObject(root.get(), "volumes");
  for (const auto& volume : volumes) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "cycle_id", volume.cycle_id.c_str());
    cJSON_AddStringToObject(item, "exercise_type",
                            volume.exercise_type.c_str());
    cJSON_AddNumberToObject(item, "total_volume_kg", volume.total_volume);
    cJSON_AddStringToObject(item, "common_original_unit",
                            volume.common_original_unit.c_str());
    cJSON_AddNumberToObject(item, "total_days", volume.total_days);
    cJSON_AddNumberToObject(item, "average_intensity_kg",
                            volume.average_intensity);
    cJSON_AddNumberToObject(item, "session_count", volume.session_count);
    cJSON_AddNumberToObject(item, "total_reps", volume.total_reps);
    cJSON_AddNumberToObject(item, "total_sets", volume.total_sets);
    cJSON_AddNumberToObject(item, "vol_power_kg", volume.vol_power);
    cJSON_AddNumberToObject(item, "vol_hypertrophy_kg",
                            volume.vol_hypertrophy);
    cJSON_AddNumberToObject(item, "vol_endurance_kg", volume.vol_endurance);
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

auto BuildImportSummaryPayload(const ImportRebuildSummary& summary)
    -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON_AddNumberToObject(root.get(), "processed_txt_count",
                          summary.processed_txt_count);
  cJSON_AddNumberToObject(root.get(), "imported_txt_count",
                          summary.imported_txt_count);
  cJSON_AddNumberToObject(root.get(), "failed_txt_count",
                          static_cast<int>(summary.failed_files.size()));
  cJSON* failed_files = cJSON_AddArrayToObject(root.get(), "failed_files");
  for (const auto& failed_file : summary.failed_files) {
    cJSON_AddItemToArray(failed_files,
                         cJSON_CreateString(failed_file.c_str()));
  }

  cJSON* failed_details = cJSON_AddArrayToObject(root.get(), "failed_details");
  for (const auto& detail : summary.failed_details) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "file", detail.file.c_str());
    cJSON_AddStringToObject(item, "stage", detail.stage.c_str());
    cJSON* diagnostics = cJSON_AddArrayToObject(item, "diagnostics");
    for (const auto& diagnostic : detail.diagnostics) {
      cJSON_AddItemToArray(diagnostics,
                           cJSON_CreateString(diagnostic.c_str()));
    }
    cJSON_AddItemToArray(failed_details, item);
  }

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto BuildArchiveExportPayload(const ArchiveExportSummary& summary)
    -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON_AddNumberToObject(root.get(), "records_count", summary.records_count_);
  cJSON_AddNumberToObject(root.get(), "json_count", summary.json_count_);
  cJSON_AddStringToObject(root.get(), "archive_output_path",
                          summary.archive_output_path_.c_str());

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto BuildArchiveImportPayload(const ArchiveImportSummary& summary)
    -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON_AddNumberToObject(root.get(), "processed_txt_count",
                          summary.processed_txt_count_);
  cJSON_AddNumberToObject(root.get(), "imported_txt_count",
                          summary.imported_txt_count_);
  cJSON_AddNumberToObject(root.get(), "failed_txt_count",
                          static_cast<int>(summary.failed_files_.size()));
  cJSON* failed_files = cJSON_AddArrayToObject(root.get(), "failed_files");
  for (const auto& failed_file : summary.failed_files_) {
    cJSON_AddItemToArray(failed_files,
                         cJSON_CreateString(failed_file.c_str()));
  }

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto BuildCycleTypeReportPayload(const std::string& cycle_id,
                                 const std::string& exercise_type,
                                 const std::string& markdown) -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  cJSON_AddStringToObject(root.get(), "cycle_id", cycle_id.c_str());
  cJSON_AddStringToObject(root.get(), "exercise_type", exercise_type.c_str());
  cJSON_AddStringToObject(root.get(), "markdown", markdown.c_str());

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto NormalizeTxtExtension(std::string extension) -> std::string {
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char ch) {
                   return static_cast<char>(std::tolower(ch));
                 });
  return extension;
}

auto CollectTxtFiles(const fs::path& root_path) -> std::vector<fs::path> {
  std::vector<fs::path> files;
  if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
    return files;
  }

  for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (NormalizeTxtExtension(entry.path().extension().string()) == ".txt") {
      files.push_back(entry.path());
    }
  }

  std::sort(files.begin(), files.end());
  return files;
}

auto DescribeRelativePath(const fs::path& root_path, const fs::path& file_path)
    -> std::string {
  std::error_code error_code;
  const fs::path relative_path = fs::relative(file_path, root_path, error_code);
  if (!error_code && !relative_path.empty()) {
    return relative_path.generic_string();
  }
  return file_path.filename().generic_string();
}

auto RemoveFileIfExists(const fs::path& path) -> void {
  std::error_code error_code;
  fs::remove(path, error_code);
}

auto TrimWhitespace(const std::string& value) -> std::string {
  const auto begin = value.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(begin, end - begin + 1);
}

template <typename Callback>
auto CaptureStdErr(Callback&& callback) -> std::string {
  std::ostringstream captured;
  auto* original = std::cerr.rdbuf(captured.rdbuf());
  try {
    callback();
  } catch (...) {
    std::cerr.rdbuf(original);
    throw;
  }
  std::cerr.rdbuf(original);
  return captured.str();
}

auto ExtractDiagnostics(const std::string& stderr_text,
                        const size_t max_lines = 12)
    -> std::vector<std::string> {
  std::vector<std::string> diagnostics;
  if (stderr_text.empty()) {
    return diagnostics;
  }

  std::istringstream stream(stderr_text);
  std::string line;
  while (std::getline(stream, line)) {
    const std::string trimmed = TrimWhitespace(line);
    if (trimmed.empty()) {
      continue;
    }
    diagnostics.push_back(trimmed);
    if (diagnostics.size() >= max_lines) {
      break;
    }
  }
  return diagnostics;
}

auto AppendDiagnostics(std::vector<std::string>& target,
                       const std::vector<std::string>& incoming) -> void {
  for (const auto& item : incoming) {
    if (item.empty()) {
      continue;
    }
    if (std::find(target.begin(), target.end(), item) != target.end()) {
      continue;
    }
    target.push_back(item);
  }
}

auto AddImportFailureDetail(ImportRebuildSummary& summary,
                            const std::string& relative_file,
                            const std::string& stage,
                            std::vector<std::string> diagnostics) -> void {
  if (diagnostics.empty()) {
    diagnostics.push_back("no diagnostics available");
  }

  summary.failed_details.push_back(
      {.file = relative_file, .stage = stage, .diagnostics = diagnostics});
}

auto ReplaceLiveDbWithStaging(const fs::path& staging_db_path,
                              const fs::path& live_db_path) -> bool {
  std::error_code error_code;
  fs::create_directories(live_db_path.parent_path(), error_code);

  const fs::path backup_db_path =
      live_db_path.parent_path() / "workout_logs.backup.sqlite3";
  RemoveFileIfExists(backup_db_path);

  const bool live_exists = fs::exists(live_db_path);
  if (live_exists) {
    fs::rename(live_db_path, backup_db_path, error_code);
    if (error_code) {
      return false;
    }
  }

  error_code.clear();
  fs::rename(staging_db_path, live_db_path, error_code);
  if (!error_code) {
    RemoveFileIfExists(backup_db_path);
    return true;
  }

  RemoveFileIfExists(live_db_path);
  if (live_exists) {
    std::error_code restore_error;
    fs::rename(backup_db_path, live_db_path, restore_error);
  }
  return false;
}

auto RebuildFromLogs(const std::string& logs_path,
                     const std::string& mapping_path,
                     const std::string& base_path) -> BridgeResult {
  const fs::path logs_root(logs_path);
  if (!fs::exists(logs_root) || !fs::is_directory(logs_root)) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
        .message = "logs_path must be an existing directory",
        .payload_json = BuildImportSummaryPayload({}),
    };
  }

  const auto files = CollectTxtFiles(logs_root);
  if (files.empty()) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
        .message = "no txt files found",
        .payload_json = BuildImportSummaryPayload({}),
    };
  }

  const fs::path live_db_path = BuildDbPath(base_path);
  const fs::path staging_db_path = BuildStagingDbPath(base_path);
  std::error_code error_code;
  fs::create_directories(staging_db_path.parent_path(), error_code);
  RemoveFileIfExists(staging_db_path);

  LogParser parser;
  FileMappingProvider mapping_provider;
  FileProcessorHandler file_processor(parser, mapping_provider);
  SqliteWorkoutRepository repository(staging_db_path.string());

  ImportRebuildSummary summary;
  for (const auto& file_path : files) {
    summary.processed_txt_count++;
    const std::string relative_label = DescribeRelativePath(logs_root, file_path);
    std::optional<std::vector<DailyData>> data_opt;
    const std::string process_stderr = CaptureStdErr([&]() {
      data_opt = file_processor.ProcessFile(
          {.file_path_ = file_path.string(), .mapping_path_ = mapping_path});
    });
    if (!data_opt.has_value()) {
      summary.failed_files.push_back(relative_label);
      auto diagnostics = ExtractDiagnostics(process_stderr);
      if (diagnostics.empty()) {
        diagnostics.push_back("validation or conversion failed");
      }
      AddImportFailureDetail(summary, relative_label, "process_file",
                             std::move(diagnostics));
      continue;
    }

    UseCaseResult<void> insert_result;
    const std::string insert_stderr = CaptureStdErr([&]() {
      insert_result = repository.InsertTrainingData(data_opt.value());
    });
    if (!insert_result.IsSuccess()) {
      summary.failed_files.push_back(relative_label);
      std::vector<std::string> diagnostics;
      if (!insert_result.message_.empty()) {
        diagnostics.push_back(insert_result.message_);
      }
      AppendDiagnostics(diagnostics, ExtractDiagnostics(insert_stderr));
      if (diagnostics.empty()) {
        diagnostics.push_back("database insert failed");
      }
      AddImportFailureDetail(summary, relative_label, "database_insert",
                             std::move(diagnostics));
      continue;
    }

    summary.imported_txt_count++;
  }

  if (summary.imported_txt_count == 0) {
    RemoveFileIfExists(staging_db_path);
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_PROCESSING_ERROR,
        .message = "no txt files imported successfully",
        .payload_json = BuildImportSummaryPayload(summary),
    };
  }

  if (!ReplaceLiveDbWithStaging(staging_db_path, live_db_path)) {
    RemoveFileIfExists(staging_db_path);
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_DATABASE_ERROR,
        .message = "failed to promote staging database",
        .payload_json = BuildImportSummaryPayload(summary),
    };
  }

  return BridgeResult{
      .status_code = summary.failed_files.empty()
                         ? WORKOUT_CORE_STATUS_SUCCESS
                         : WORKOUT_CORE_STATUS_PROCESSING_ERROR,
      .message = summary.failed_files.empty() ? "" : "partial import completed",
      .payload_json = BuildImportSummaryPayload(summary),
  };
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

auto QueryCycles(const std::string& base_path) -> BridgeResult {
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

  auto cycles = QueryFacade::GetAllCycles(db_manager.GetConnection());
  return BridgeResult{
      .status_code = WORKOUT_CORE_STATUS_SUCCESS,
      .message = "",
      .payload_json = BuildCyclesPayload(cycles),
  };
}

auto QueryCycleVolumes(const std::string& base_path, const std::string& cycle_id)
    -> BridgeResult {
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

  auto volumes =
      QueryFacade::GetVolumeStatsByCycle(db_manager.GetConnection(), cycle_id);
  return BridgeResult{
      .status_code = WORKOUT_CORE_STATUS_SUCCESS,
      .message = "",
      .payload_json = BuildCycleVolumesPayload(volumes),
  };
}

auto QueryCycleTypeReport(const std::string& base_path,
                          const std::string& cycle_id,
                          const std::string& exercise_type,
                          const std::string& display_unit) -> BridgeResult {
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

  auto markdown = ReportFacade::GenerateTypeReportMarkdown(
      db_manager.GetConnection(), cycle_id, exercise_type, display_unit);
  if (!markdown.has_value() || markdown->empty()) {
    return BridgeResult{
        .status_code = WORKOUT_CORE_STATUS_PROCESSING_ERROR,
        .message = "report markdown not found",
        .payload_json = "{}",
    };
  }

  return BridgeResult{
      .status_code = WORKOUT_CORE_STATUS_SUCCESS,
      .message = "",
      .payload_json =
          BuildCycleTypeReportPayload(cycle_id, exercise_type, markdown.value()),
  };
}

auto ExportArchive(const std::string& base_path,
                   const std::string& archive_output_path) -> BridgeResult {
  LogParser parser;
  FileMappingProvider mapping_provider;
  ZipArchiveService archive_service(parser, mapping_provider);

  auto export_result = archive_service.ExportArchive({
      .base_path_ = base_path,
      .archive_output_path_ = archive_output_path,
  });
  if (!export_result.IsSuccess()) {
    return BridgeResult{
        .status_code = MapCoreErrorCode(export_result.error_code_),
        .message = export_result.message_,
        .payload_json = "{}",
    };
  }

  return BridgeResult{
      .status_code = WORKOUT_CORE_STATUS_SUCCESS,
      .message = "",
      .payload_json = BuildArchiveExportPayload(export_result.payload_.value()),
  };
}

auto ImportArchive(const std::string& base_path, const std::string& archive_path)
    -> BridgeResult {
  LogParser parser;
  FileMappingProvider mapping_provider;
  ZipArchiveService archive_service(parser, mapping_provider);

  auto import_result = archive_service.ImportArchive({
      .base_path_ = base_path,
      .archive_path_ = archive_path,
  });
  if (!import_result.IsSuccess()) {
    return BridgeResult{
        .status_code = MapCoreErrorCode(import_result.error_code_),
        .message = import_result.message_,
        .payload_json = "{}",
    };
  }

  const auto& summary = import_result.payload_.value();
  return BridgeResult{
      .status_code =
          summary.imported_txt_count_ > 0 && summary.failed_files_.empty()
              ? WORKOUT_CORE_STATUS_SUCCESS
              : WORKOUT_CORE_STATUS_PROCESSING_ERROR,
      .message =
          summary.imported_txt_count_ > 0 && summary.failed_files_.empty()
              ? ""
              : (summary.imported_txt_count_ > 0
                     ? "partial archive import completed"
                     : "archive import failed"),
      .payload_json = BuildArchiveImportPayload(summary),
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
Java_com_workout_calculator_core_WorkoutNativeBridge_rebuildFromLogs(
    JNIEnv* env, jobject /*thiz*/, jstring logs_path, jstring mapping_path,
    jstring base_path) {
  const std::string logs_path_text = FromJString(env, logs_path);
  const std::string mapping_path_text = FromJString(env, mapping_path);
  const std::string base_path_text = FromJString(env, base_path);

  if (logs_path_text.empty() || mapping_path_text.empty() ||
      base_path_text.empty()) {
    return MakeNativeResultObject(
        env,
        BridgeResult{
            .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
            .message = "logs_path, mapping_path and base_path are required",
            .payload_json = BuildImportSummaryPayload({}),
        });
  }

  return MakeNativeResultObject(
      env, RebuildFromLogs(logs_path_text, mapping_path_text, base_path_text));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_exportArchive(
    JNIEnv* env, jobject /*thiz*/, jstring base_path,
    jstring archive_output_path) {
  const std::string base_path_text = FromJString(env, base_path);
  const std::string archive_output_path_text =
      FromJString(env, archive_output_path);

  if (base_path_text.empty() || archive_output_path_text.empty()) {
    return MakeNativeResultObject(
        env, BridgeResult{
                 .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
                 .message = "base_path and archive_output_path are required",
                 .payload_json = "{}",
             });
  }

  return MakeNativeResultObject(
      env, ExportArchive(base_path_text, archive_output_path_text));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_importArchive(
    JNIEnv* env, jobject /*thiz*/, jstring base_path, jstring archive_path) {
  const std::string base_path_text = FromJString(env, base_path);
  const std::string archive_path_text = FromJString(env, archive_path);

  if (base_path_text.empty() || archive_path_text.empty()) {
    return MakeNativeResultObject(
        env, BridgeResult{
                 .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
                 .message = "base_path and archive_path are required",
                 .payload_json = "{}",
             });
  }

  return MakeNativeResultObject(env,
                                ImportArchive(base_path_text, archive_path_text));
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

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_queryCycles(
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

  return MakeNativeResultObject(env, QueryCycles(base_path_text));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_queryCycleVolumes(
    JNIEnv* env, jobject /*thiz*/, jstring base_path, jstring cycle_id) {
  const std::string base_path_text = FromJString(env, base_path);
  const std::string cycle_id_text = FromJString(env, cycle_id);

  if (base_path_text.empty() || cycle_id_text.empty()) {
    return MakeNativeResultObject(
        env, BridgeResult{
                 .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
                 .message = "base_path and cycle_id are required",
                 .payload_json = "{}",
             });
  }

  return MakeNativeResultObject(env,
                                QueryCycleVolumes(base_path_text, cycle_id_text));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_workout_calculator_core_WorkoutNativeBridge_queryCycleTypeReport(
    JNIEnv* env, jobject /*thiz*/, jstring base_path, jstring cycle_id,
    jstring exercise_type, jstring display_unit) {
  const std::string base_path_text = FromJString(env, base_path);
  const std::string cycle_id_text = FromJString(env, cycle_id);
  const std::string exercise_type_text = FromJString(env, exercise_type);
  const std::string display_unit_text = FromJString(env, display_unit);

  if (base_path_text.empty() || cycle_id_text.empty() ||
      exercise_type_text.empty() || display_unit_text.empty()) {
    return MakeNativeResultObject(
        env, BridgeResult{
                 .status_code = WORKOUT_CORE_STATUS_INVALID_ARGS,
                 .message =
                     "base_path, cycle_id, exercise_type and display_unit are required",
                 .payload_json = "{}",
             });
  }

  return MakeNativeResultObject(
      env, QueryCycleTypeReport(base_path_text, cycle_id_text,
                                exercise_type_text, display_unit_text));
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
