#include "infrastructure/archive/zip_archive_service.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "application/file_processor_handler.hpp"
#include "common/c_json_helper.hpp"
#include "common/version.hpp"
#include "core/application/core_error_code.hpp"
#include "infrastructure/persistence/repository/sqlite_workout_repository.hpp"
#include "infrastructure/serializer/serializer.hpp"
#include "miniz.h"

namespace fs = std::filesystem;

namespace {

constexpr char kArchiveType[] = "workout_backup";
constexpr int kArchiveVersion = 1;
constexpr char kManifestFileName[] = "manifest.json";
constexpr char kRecordsDirName[] = "records";
constexpr char kDataDirName[] = "data";
constexpr char kConfigDirName[] = "config";
constexpr char kMappingFileName[] = "mapping.toml";
constexpr char kMappingArchivePath[] = "config/mapping.toml";

struct SnapshotEntry {
  fs::path source_txt_path;
  fs::path relative_txt_path;
  fs::path generated_json_path;
};

struct PromotionStep {
  fs::path staging_path;
  fs::path live_path;
  fs::path backup_path;
  bool live_existed = false;
};

auto NormalizeTxtExtension(std::string extension) -> std::string {
  for (char& ch : extension) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
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

auto BuildLiveRecordsPath(const fs::path& base_path) -> fs::path {
  return base_path / "records_live";
}

auto BuildConfigDirPath(const fs::path& base_path) -> fs::path {
  return base_path / kConfigDirName;
}

auto BuildLiveMappingPath(const fs::path& base_path) -> fs::path {
  return BuildConfigDirPath(base_path) / kMappingFileName;
}

auto BuildLiveDataPath(const fs::path& base_path) -> fs::path {
  return base_path / "output" / "data";
}

auto BuildLiveDbPath(const fs::path& base_path) -> fs::path {
  return base_path / "output" / "db" / "workout_logs.sqlite3";
}

auto BuildStagingDbPath(const fs::path& base_path) -> fs::path {
  return base_path / "output" / "db" / "workout_logs.staging.sqlite3";
}

auto BuildArchiveTempDir(const fs::path& base_path) -> fs::path {
  return base_path / "archive_tmp";
}

auto BuildArchiveDataStagingPath(const fs::path& base_path) -> fs::path {
  return BuildArchiveTempDir(base_path) / "data_staging";
}

auto BuildArchiveExtractStagingPath(const fs::path& base_path) -> fs::path {
  return base_path / "archive_extract_staging";
}

auto RemovePathIfExists(const fs::path& path) -> void {
  std::error_code error_code;
  fs::remove_all(path, error_code);
  error_code.clear();
  fs::remove(path, error_code);
}

auto EnsureCleanDirectory(const fs::path& path) -> bool {
  RemovePathIfExists(path);
  std::error_code error_code;
  fs::create_directories(path, error_code);
  return !error_code && fs::exists(path) && fs::is_directory(path);
}

auto EnsureParentDirectory(const fs::path& path) -> bool {
  const fs::path parent = path.parent_path();
  if (parent.empty()) {
    return true;
  }
  std::error_code error_code;
  fs::create_directories(parent, error_code);
  return !error_code;
}

auto WriteTextFile(const fs::path& path, const std::string& content) -> bool {
  if (!EnsureParentDirectory(path)) {
    return false;
  }

  std::ofstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return false;
  }
  file << content;
  return file.good();
}

auto ReadTextFile(const fs::path& path) -> std::optional<std::string> {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

auto BuildJsonRelativePath(const fs::path& relative_txt_path) -> fs::path {
  fs::path json_relative_path = relative_txt_path;
  json_relative_path.replace_extension(".json");
  return json_relative_path;
}

auto BuildTimestampUtc() -> std::string {
  const auto now = std::chrono::system_clock::now();
  const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm utc_time{};
#if defined(_WIN32)
  gmtime_s(&utc_time, &now_time_t);
#else
  gmtime_r(&now_time_t, &utc_time);
#endif

  char buffer[32] = {};
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc_time);
  return buffer;
}

auto GetJsonString(const cJSON* object, const char* key) -> std::string {
  if (object == nullptr || key == nullptr) {
    return "";
  }

  cJSON* item = cJSON_GetObjectItemCaseSensitive(const_cast<cJSON*>(object), key);
  if (cJSON_IsString(item) != 0 && item->valuestring != nullptr) {
    return item->valuestring;
  }
  return "";
}

auto GetJsonInt(const cJSON* object, const char* key, int default_value = 0)
    -> int {
  if (object == nullptr || key == nullptr) {
    return default_value;
  }

  cJSON* item = cJSON_GetObjectItemCaseSensitive(const_cast<cJSON*>(object), key);
  if (cJSON_IsNumber(item) != 0) {
    return item->valueint;
  }
  return default_value;
}

auto BuildManifestJson(int records_count, int json_count) -> std::string {
  CJsonPtr root = MakeCJson(cJSON_CreateObject());
  if (root == nullptr) {
    return "{}";
  }

  const std::string core_version(BuildInfo::VERSION);

  cJSON_AddStringToObject(root.get(), "archive_type", kArchiveType);
  cJSON_AddNumberToObject(root.get(), "archive_version", kArchiveVersion);
  cJSON_AddStringToObject(root.get(), "exported_at",
                          BuildTimestampUtc().c_str());
  cJSON_AddStringToObject(root.get(), "core_version", core_version.c_str());
  cJSON_AddNumberToObject(root.get(), "records_count", records_count);
  cJSON_AddNumberToObject(root.get(), "json_count", json_count);
  cJSON_AddStringToObject(root.get(), "mapping_path", kMappingArchivePath);

  char* json_chars = cJSON_PrintUnformatted(root.get());
  if (json_chars == nullptr) {
    return "{}";
  }

  std::string payload(json_chars);
  cJSON_free(json_chars);
  return payload;
}

auto ValidateManifest(const fs::path& manifest_path) -> UseCaseResult<void> {
  const auto manifest_content = ReadTextFile(manifest_path);
  if (!manifest_content.has_value()) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kFileNotFound,
                                        "archive manifest missing");
  }

  CJsonPtr root = MakeCJson(cJSON_Parse(manifest_content->c_str()));
  if (root == nullptr || cJSON_IsObject(root.get()) == 0) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                        "archive manifest is invalid json");
  }

  if (GetJsonString(root.get(), "archive_type") != kArchiveType) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                        "archive_type is invalid");
  }

  if (GetJsonInt(root.get(), "archive_version") != kArchiveVersion) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                        "archive_version is invalid");
  }

  if (GetJsonString(root.get(), "mapping_path") != kMappingArchivePath) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                        "mapping_path is invalid");
  }

  return UseCaseResult<void>::Success();
}

auto AddFileToZip(mz_zip_archive& archive, const std::string& archive_path,
                  const fs::path& source_path) -> bool {
  return mz_zip_writer_add_file(&archive, archive_path.c_str(),
                                source_path.string().c_str(), nullptr, 0,
                                MZ_BEST_COMPRESSION) != 0;
}

auto AddStringToZip(mz_zip_archive& archive, const std::string& archive_path,
                    const std::string& content) -> bool {
  return mz_zip_writer_add_mem(&archive, archive_path.c_str(), content.data(),
                               content.size(),
                               MZ_BEST_COMPRESSION) != 0;
}

auto IsSafeArchiveEntryPath(const std::string& entry_name) -> bool {
  if (entry_name.empty()) {
    return false;
  }

  const fs::path entry_path(entry_name);
  if (entry_path.is_absolute()) {
    return false;
  }

  for (const auto& part : entry_path) {
    const std::string token = part.generic_string();
    if (token == "..") {
      return false;
    }
  }

  return true;
}

auto CopyDirectoryRecursively(const fs::path& source, const fs::path& target)
    -> bool {
  if (!fs::exists(source)) {
    return false;
  }

  std::error_code error_code;
  fs::create_directories(target, error_code);
  if (error_code) {
    return false;
  }

  for (const auto& entry : fs::recursive_directory_iterator(source)) {
    const fs::path relative_path = fs::relative(entry.path(), source, error_code);
    if (error_code) {
      return false;
    }

    const fs::path destination = target / relative_path;
    if (entry.is_directory()) {
      fs::create_directories(destination, error_code);
      if (error_code) {
        return false;
      }
      continue;
    }

    if (!EnsureParentDirectory(destination)) {
      return false;
    }

    fs::copy_file(entry.path(), destination, fs::copy_options::overwrite_existing,
                  error_code);
    if (error_code) {
      return false;
    }
  }

  return true;
}

auto PromotePath(const fs::path& staging_path, const fs::path& live_path,
                 const fs::path& backup_path, PromotionStep* step) -> bool {
  if (step == nullptr) {
    return false;
  }

  step->staging_path = staging_path;
  step->live_path = live_path;
  step->backup_path = backup_path;
  step->live_existed = fs::exists(live_path);

  RemovePathIfExists(backup_path);
  if (step->live_existed) {
    if (!EnsureParentDirectory(backup_path)) {
      return false;
    }
    std::error_code rename_error;
    fs::rename(live_path, backup_path, rename_error);
    if (rename_error) {
      return false;
    }
  }

  if (!EnsureParentDirectory(live_path)) {
    if (step->live_existed && fs::exists(backup_path)) {
      std::error_code restore_error;
      fs::rename(backup_path, live_path, restore_error);
    }
    return false;
  }

  std::error_code rename_error;
  fs::rename(staging_path, live_path, rename_error);
  if (!rename_error) {
    return true;
  }

  bool copied = false;
  if (fs::is_directory(staging_path)) {
    copied = CopyDirectoryRecursively(staging_path, live_path);
    if (copied) {
      RemovePathIfExists(staging_path);
    }
  } else if (fs::is_regular_file(staging_path)) {
    std::error_code copy_error;
    fs::copy_file(staging_path, live_path, fs::copy_options::overwrite_existing,
                  copy_error);
    copied = !copy_error;
    if (copied) {
      RemovePathIfExists(staging_path);
    }
  }

  if (copied) {
    return true;
  }

  RemovePathIfExists(live_path);
  if (step->live_existed && fs::exists(backup_path)) {
    std::error_code restore_error;
    fs::rename(backup_path, live_path, restore_error);
  }
  return false;
}

auto RollbackPromotions(std::vector<PromotionStep>* steps) -> void {
  if (steps == nullptr) {
    return;
  }

  for (auto it = steps->rbegin(); it != steps->rend(); ++it) {
    RemovePathIfExists(it->live_path);
    if (it->live_existed && fs::exists(it->backup_path)) {
      std::error_code restore_error;
      fs::rename(it->backup_path, it->live_path, restore_error);
    }
  }
}

auto CleanupPromotions(const std::vector<PromotionStep>& steps) -> void {
  for (const auto& step : steps) {
    RemovePathIfExists(step.backup_path);
  }
}

auto BuildArchiveSnapshot(const fs::path& records_root,
                          const fs::path& mapping_path,
                          const fs::path& generated_data_dir,
                          ILogParser& parser,
                          IMappingProvider& mapping_provider)
    -> UseCaseResult<std::vector<SnapshotEntry>> {
  if (!fs::exists(records_root) || !fs::is_directory(records_root)) {
    return UseCaseResult<std::vector<SnapshotEntry>>::Failure(
        CoreErrorCode::kFileNotFound, "records_live directory not found");
  }

  if (!fs::exists(mapping_path) || !fs::is_regular_file(mapping_path)) {
    return UseCaseResult<std::vector<SnapshotEntry>>::Failure(
        CoreErrorCode::kFileNotFound, "mapping.toml not found");
  }

  const auto files = CollectTxtFiles(records_root);
  if (files.empty()) {
    return UseCaseResult<std::vector<SnapshotEntry>>::Failure(
        CoreErrorCode::kInvalidArgs, "no txt files found in records_live");
  }

  if (!EnsureCleanDirectory(generated_data_dir)) {
    return UseCaseResult<std::vector<SnapshotEntry>>::Failure(
        CoreErrorCode::kProcessingError,
        "failed to prepare json staging directory");
  }

  FileProcessorHandler file_processor(parser, mapping_provider);
  std::vector<SnapshotEntry> entries;
  entries.reserve(files.size());

  for (const auto& file_path : files) {
    auto data_opt = file_processor.ProcessFile(
        {.file_path_ = file_path.string(), .mapping_path_ = mapping_path.string()});
    if (!data_opt.has_value()) {
      return UseCaseResult<std::vector<SnapshotEntry>>::Failure(
          CoreErrorCode::kProcessingError,
          "failed to convert txt file: " + DescribeRelativePath(records_root, file_path));
    }

    const fs::path relative_txt_path = fs::relative(file_path, records_root);
    const fs::path json_output_path =
        generated_data_dir / BuildJsonRelativePath(relative_txt_path);
    if (!WriteTextFile(json_output_path, Serializer::Serialize(data_opt.value()))) {
      return UseCaseResult<std::vector<SnapshotEntry>>::Failure(
          CoreErrorCode::kProcessingError,
          "failed to write json snapshot: " +
              DescribeRelativePath(records_root, file_path));
    }

    entries.push_back({
        .source_txt_path = file_path,
        .relative_txt_path = relative_txt_path,
        .generated_json_path = json_output_path,
    });
  }

  return UseCaseResult<std::vector<SnapshotEntry>>::Success(std::move(entries));
}

auto WriteArchiveBundle(const fs::path& archive_output_path,
                        const fs::path& mapping_path,
                        const std::vector<SnapshotEntry>& entries)
    -> UseCaseResult<void> {
  if (!EnsureParentDirectory(archive_output_path)) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                        "failed to prepare archive directory");
  }

  RemovePathIfExists(archive_output_path);

  mz_zip_archive archive{};
  if (mz_zip_writer_init_file(&archive, archive_output_path.string().c_str(), 0) ==
      0) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                        "failed to create archive file");
  }

  const auto end_writer = [&archive]() {
    mz_zip_writer_end(&archive);
  };

  const std::string manifest_json =
      BuildManifestJson(static_cast<int>(entries.size()),
                        static_cast<int>(entries.size()));
  if (!AddStringToZip(archive, kManifestFileName, manifest_json) ||
      !AddFileToZip(archive, kMappingArchivePath, mapping_path)) {
    end_writer();
    RemovePathIfExists(archive_output_path);
    return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                        "failed to write archive metadata");
  }

  for (const auto& entry : entries) {
    const std::string records_archive_path =
        (fs::path(kRecordsDirName) / entry.relative_txt_path).generic_string();
    const std::string data_archive_path =
        (fs::path(kDataDirName) / BuildJsonRelativePath(entry.relative_txt_path))
            .generic_string();

    if (!AddFileToZip(archive, records_archive_path, entry.source_txt_path) ||
        !AddFileToZip(archive, data_archive_path, entry.generated_json_path)) {
      end_writer();
      RemovePathIfExists(archive_output_path);
      return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                          "failed to write archive entries");
    }
  }

  if (mz_zip_writer_finalize_archive(&archive) == 0) {
    end_writer();
    RemovePathIfExists(archive_output_path);
    return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                        "failed to finalize archive");
  }

  end_writer();
  return UseCaseResult<void>::Success();
}

auto ExtractArchive(const fs::path& archive_path, const fs::path& extract_root)
    -> UseCaseResult<void> {
  if (!fs::exists(archive_path) || !fs::is_regular_file(archive_path)) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kFileNotFound,
                                        "archive file not found");
  }

  if (!EnsureCleanDirectory(extract_root)) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                        "failed to prepare extract staging");
  }

  mz_zip_archive archive{};
  if (mz_zip_reader_init_file(&archive, archive_path.string().c_str(), 0) == 0) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                        "failed to open archive");
  }

  const auto end_reader = [&archive]() {
    mz_zip_reader_end(&archive);
  };

  const mz_uint file_count = mz_zip_reader_get_num_files(&archive);
  for (mz_uint index = 0; index < file_count; ++index) {
    mz_zip_archive_file_stat file_stat{};
    if (mz_zip_reader_file_stat(&archive, index, &file_stat) == 0) {
      end_reader();
      return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                          "failed to read archive entry stat");
    }

    const std::string entry_name(file_stat.m_filename);
    if (!IsSafeArchiveEntryPath(entry_name)) {
      end_reader();
      return UseCaseResult<void>::Failure(CoreErrorCode::kValidationError,
                                          "archive contains unsafe entry path");
    }

    const fs::path destination_path = extract_root / fs::path(entry_name);
    if (mz_zip_reader_is_file_a_directory(&archive, index) != 0) {
      std::error_code error_code;
      fs::create_directories(destination_path, error_code);
      if (error_code) {
        end_reader();
        return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                            "failed to create extracted directory");
      }
      continue;
    }

    if (!EnsureParentDirectory(destination_path) ||
        mz_zip_reader_extract_to_file(&archive, index,
                                      destination_path.string().c_str(), 0) == 0) {
      end_reader();
      return UseCaseResult<void>::Failure(CoreErrorCode::kProcessingError,
                                          "failed to extract archive file");
    }
  }

  end_reader();
  return UseCaseResult<void>::Success();
}

auto ImportArchiveRecords(const fs::path& records_root,
                          const fs::path& mapping_path,
                          const fs::path& staging_db_path,
                          const fs::path& generated_data_dir,
                          ILogParser& parser,
                          IMappingProvider& mapping_provider)
    -> UseCaseResult<ArchiveImportSummary> {
  if (!fs::exists(records_root) || !fs::is_directory(records_root)) {
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kValidationError, "archive records directory missing");
  }

  if (!fs::exists(mapping_path) || !fs::is_regular_file(mapping_path)) {
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kValidationError, "archive mapping.toml missing");
  }

  const auto files = CollectTxtFiles(records_root);
  if (files.empty()) {
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kInvalidArgs, "archive contains no txt files");
  }

  if (!EnsureCleanDirectory(generated_data_dir)) {
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kProcessingError,
        "failed to prepare imported json staging directory");
  }

  if (!EnsureParentDirectory(staging_db_path)) {
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kProcessingError, "failed to prepare database directory");
  }
  RemovePathIfExists(staging_db_path);

  FileProcessorHandler file_processor(parser, mapping_provider);
  SqliteWorkoutRepository repository(staging_db_path.string());
  ArchiveImportSummary summary;

  for (const auto& file_path : files) {
    summary.processed_txt_count_++;
    const std::string relative_label = DescribeRelativePath(records_root, file_path);
    auto data_opt = file_processor.ProcessFile(
        {.file_path_ = file_path.string(), .mapping_path_ = mapping_path.string()});
    if (!data_opt.has_value()) {
      summary.failed_files_.push_back(relative_label);
      continue;
    }

    const fs::path relative_txt_path = fs::relative(file_path, records_root);
    const fs::path generated_json_path =
        generated_data_dir / BuildJsonRelativePath(relative_txt_path);
    if (!WriteTextFile(generated_json_path, Serializer::Serialize(data_opt.value()))) {
      summary.failed_files_.push_back(relative_label);
      continue;
    }

    auto insert_result = repository.InsertTrainingData(data_opt.value());
    if (!insert_result.IsSuccess()) {
      RemovePathIfExists(generated_json_path);
      summary.failed_files_.push_back(relative_label);
      continue;
    }

    summary.imported_txt_count_++;
  }

  if (summary.imported_txt_count_ == 0) {
    RemovePathIfExists(staging_db_path);
  }

  return UseCaseResult<ArchiveImportSummary>::Success(std::move(summary));
}

auto PromoteImportedSnapshot(const fs::path& base_path,
                             const fs::path& staging_records_path,
                             const fs::path& staging_config_path,
                             const fs::path& staging_data_path,
                             const fs::path& staging_db_path) -> bool {
  std::vector<PromotionStep> steps;
  steps.reserve(4);

  PromotionStep records_step;
  if (!PromotePath(staging_records_path, BuildLiveRecordsPath(base_path),
                   base_path / "records_backup", &records_step)) {
    return false;
  }
  steps.push_back(records_step);

  PromotionStep config_step;
  if (!PromotePath(staging_config_path, BuildConfigDirPath(base_path),
                   base_path / "config_backup", &config_step)) {
    RollbackPromotions(&steps);
    return false;
  }
  steps.push_back(config_step);

  PromotionStep data_step;
  if (!PromotePath(staging_data_path, BuildLiveDataPath(base_path),
                   base_path / "output" / "data_backup", &data_step)) {
    RollbackPromotions(&steps);
    return false;
  }
  steps.push_back(data_step);

  PromotionStep db_step;
  if (!PromotePath(staging_db_path, BuildLiveDbPath(base_path),
                   base_path / "output" / "db" / "workout_logs.backup.sqlite3",
                   &db_step)) {
    RollbackPromotions(&steps);
    return false;
  }
  steps.push_back(db_step);

  CleanupPromotions(steps);
  return true;
}

}  // namespace

ZipArchiveService::ZipArchiveService(ILogParser& parser,
                                     IMappingProvider& mapping_provider)
    : parser_(parser), mapping_provider_(mapping_provider) {}

auto ZipArchiveService::ExportArchive(const ArchiveExportRequest& request)
    -> UseCaseResult<ArchiveExportSummary> {
  if (request.base_path_.empty() || request.archive_output_path_.empty()) {
    return UseCaseResult<ArchiveExportSummary>::Failure(
        CoreErrorCode::kInvalidArgs,
        "base_path and archive_output_path are required");
  }

  const fs::path base_path(request.base_path_);
  const fs::path records_root = BuildLiveRecordsPath(base_path);
  const fs::path mapping_path = BuildLiveMappingPath(base_path);
  const fs::path generated_data_dir = BuildArchiveDataStagingPath(base_path);

  auto snapshot_result =
      BuildArchiveSnapshot(records_root, mapping_path, generated_data_dir, parser_,
                           mapping_provider_);
  if (!snapshot_result.IsSuccess()) {
    return UseCaseResult<ArchiveExportSummary>::Failure(
        snapshot_result.error_code_, snapshot_result.message_);
  }

  const auto& entries = snapshot_result.payload_.value();
  auto archive_result =
      WriteArchiveBundle(request.archive_output_path_, mapping_path, entries);
  if (!archive_result.IsSuccess()) {
    RemovePathIfExists(generated_data_dir);
    return UseCaseResult<ArchiveExportSummary>::Failure(
        archive_result.error_code_, archive_result.message_);
  }

  PromotionStep data_step;
  if (!PromotePath(generated_data_dir, BuildLiveDataPath(base_path),
                   base_path / "output" / "data_backup", &data_step)) {
    RemovePathIfExists(request.archive_output_path_);
    return UseCaseResult<ArchiveExportSummary>::Failure(
        CoreErrorCode::kProcessingError, "failed to promote json snapshot");
  }
  CleanupPromotions(std::vector<PromotionStep>{data_step});

  return UseCaseResult<ArchiveExportSummary>::Success({
      .records_count_ = static_cast<int>(entries.size()),
      .json_count_ = static_cast<int>(entries.size()),
      .archive_output_path_ = request.archive_output_path_,
  });
}

auto ZipArchiveService::ImportArchive(const ArchiveImportRequest& request)
    -> UseCaseResult<ArchiveImportSummary> {
  if (request.base_path_.empty() || request.archive_path_.empty()) {
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kInvalidArgs, "base_path and archive_path are required");
  }

  const fs::path base_path(request.base_path_);
  const fs::path extract_root = BuildArchiveExtractStagingPath(base_path);
  auto extract_result = ExtractArchive(request.archive_path_, extract_root);
  if (!extract_result.IsSuccess()) {
    RemovePathIfExists(extract_root);
    return UseCaseResult<ArchiveImportSummary>::Failure(
        extract_result.error_code_, extract_result.message_);
  }

  auto manifest_result = ValidateManifest(extract_root / kManifestFileName);
  if (!manifest_result.IsSuccess()) {
    RemovePathIfExists(extract_root);
    return UseCaseResult<ArchiveImportSummary>::Failure(
        manifest_result.error_code_, manifest_result.message_);
  }

  const fs::path records_path = extract_root / kRecordsDirName;
  const fs::path config_path = extract_root / kConfigDirName;
  const fs::path mapping_path = config_path / kMappingFileName;
  const fs::path extracted_data_path = extract_root / kDataDirName;
  const fs::path generated_data_dir = extract_root / "data_generated";
  const fs::path staging_db_path = BuildStagingDbPath(base_path);

  if (!fs::exists(extracted_data_path) || !fs::is_directory(extracted_data_path)) {
    RemovePathIfExists(extract_root);
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kValidationError, "archive data directory missing");
  }

  auto import_result = ImportArchiveRecords(records_path, mapping_path,
                                            staging_db_path, generated_data_dir,
                                            parser_, mapping_provider_);
  if (!import_result.IsSuccess()) {
    RemovePathIfExists(extract_root);
    RemovePathIfExists(staging_db_path);
    return UseCaseResult<ArchiveImportSummary>::Failure(
        import_result.error_code_, import_result.message_);
  }

  ArchiveImportSummary summary = import_result.payload_.value();
  if (summary.imported_txt_count_ == 0) {
    RemovePathIfExists(extract_root);
    return UseCaseResult<ArchiveImportSummary>::Success(std::move(summary));
  }

  if (!PromoteImportedSnapshot(base_path, records_path, config_path,
                               generated_data_dir, staging_db_path)) {
    RemovePathIfExists(extract_root);
    RemovePathIfExists(staging_db_path);
    return UseCaseResult<ArchiveImportSummary>::Failure(
        CoreErrorCode::kProcessingError,
        "failed to promote imported archive snapshot");
  }

  RemovePathIfExists(extract_root);
  return UseCaseResult<ArchiveImportSummary>::Success(std::move(summary));
}
