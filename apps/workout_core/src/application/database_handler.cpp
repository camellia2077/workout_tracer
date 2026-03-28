// application/database_handler.cpp

#include "application/database_handler.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

#include "common/file_reader.hpp"
#include "common/json_reader.hpp"
#include "core/application/core_error_code.hpp"
#include "core/application/use_case_result.hpp"
#include "domain/services/training_metrics_service.hpp"
#include "infrastructure/serializer/serializer.hpp"

namespace fs = std::filesystem;

namespace {

auto ResolveDatabasePath(const AppConfig& config) -> fs::path {
  return fs::path(config.base_path_) / "output" / "db" / "workout_logs.sqlite3";
}

auto ResolveDatabaseDirectory(const AppConfig& config) -> fs::path {
  return ResolveDatabasePath(config).parent_path();
}

auto ResolveReportOutputDir(const AppConfig& config) -> fs::path {
  return fs::path(config.base_path_) / "output" / "reports";
}

auto MapCoreErrorCode(CoreErrorCode error_code) -> AppExitCode {
  switch (error_code) {
    case CoreErrorCode::kSuccess:
      return AppExitCode::kSuccess;
    case CoreErrorCode::kInvalidArgs:
      return AppExitCode::kInvalidArgs;
    case CoreErrorCode::kValidationError:
      return AppExitCode::kValidationError;
    case CoreErrorCode::kFileNotFound:
      return AppExitCode::kFileNotFound;
    case CoreErrorCode::kDatabaseError:
      return AppExitCode::kDatabaseError;
    case CoreErrorCode::kProcessingError:
      return AppExitCode::kProcessingError;
    case CoreErrorCode::kNotImplemented:
    case CoreErrorCode::kUnknownError:
    default:
      return AppExitCode::kUnknownError;
  }
}

template <typename PayloadType>
auto ToExitCode(const UseCaseResult<PayloadType>& result) -> AppExitCode {
  return MapCoreErrorCode(result.error_code_);
}

auto PrintPersonalRecords(const std::vector<WorkoutPersonalRecord>& prs)
    -> void {
  std::cout << "\n--- Personal Records ---" << std::endl;
  for (const auto& pr : prs) {
    std::cout << pr.exercise_name_ << ": " << pr.max_weight_ << "kg x "
              << pr.reps_ << " (Date: " << pr.date_ << ")";
    if (pr.reps_ > 1) {
      std::cout << " [Est. 1RM: Epley " << std::fixed << std::setprecision(1)
                << pr.estimated_1rm_epley_ << "kg, Brzycki "
                << pr.estimated_1rm_brzycki_ << "kg]";
    }
    std::cout << std::endl;
  }
}

auto PrintExercises(const std::vector<WorkoutExerciseInfo>& exercises) -> void {
  std::cout << "\n--- Exercises ---" << std::endl;
  for (const auto& info : exercises) {
    std::cout << "- " << info.name_ << " [" << info.type_ << "]" << std::endl;
  }
}

auto PrintCycles(const std::vector<WorkoutCycleRecord>& cycles) -> void {
  std::cout << "\n--- Training Cycles ---" << std::endl;
  for (const auto& cycle : cycles) {
    std::cout << "Cycle: " << cycle.cycle_id_ << " [" << cycle.type_ << "] "
              << "Duration: " << cycle.total_days_ << " days ("
              << cycle.start_date_ << " to " << cycle.end_date_ << ")"
              << std::endl;
  }
}

auto PrintVolumeStats(const WorkoutVolumeStats& stats) -> void {
  const double avg_daily_vol = TrainingMetricsService::AverageDailyVolume(
      stats.total_volume_, stats.total_days_);
  const double frequency = TrainingMetricsService::SessionsPerWeek(
      stats.session_count_, stats.total_days_);
  const double density =
      TrainingMetricsService::SetsPerDay(stats.total_sets_, stats.total_days_);

  auto get_percent = [&](double vol) -> double {
    return TrainingMetricsService::PercentageOfTotal(vol, stats.total_volume_);
  };

  std::cout << "\n--- Advanced Kinematics Analysis ---" << std::endl;
  std::cout << "Cycle:           " << stats.cycle_id_ << std::endl;
  std::cout << "Type:            " << stats.exercise_type_ << std::endl;
  std::cout << "------------------------------------" << std::endl;
  std::cout << "Total Volume:    " << std::fixed << std::setprecision(1)
            << stats.total_volume_ << "kg" << std::endl;
  std::cout << "Avg Intensity:   " << stats.average_intensity_ << "kg/rep"
            << std::endl;
  std::cout << "Avg Daily Vol:   " << avg_daily_vol << "kg/day" << std::endl;
  std::cout << "------------------------------------" << std::endl;
  std::cout << "Sessions:        " << stats.session_count_ << std::endl;
  std::cout << "Frequency:       " << frequency << " sessions/week"
            << std::endl;
  std::cout << "Density:         " << density << " sets/day" << std::endl;
  std::cout << "------------------------------------" << std::endl;
  std::cout << "Intensity Distribution:" << std::endl;
  std::cout << "  Power (1-5):   " << get_percent(stats.vol_power_) << "%"
            << std::endl;
  std::cout << "  Hyper (6-12):  " << get_percent(stats.vol_hypertrophy_) << "%"
            << std::endl;
  std::cout << "  Endure (13+):  " << get_percent(stats.vol_endurance_) << "%"
            << std::endl;
}

}  // namespace

DatabaseHandler::DatabaseHandler(IWorkoutRepository& repository,
                                 IReportExporter& report_exporter)
    : repository_(repository), report_exporter_(report_exporter) {}

auto DatabaseHandler::Handle(const AppConfig& config) -> AppExitCode {
  if (config.action_ == ActionType::Insert) {
    std::cout << "Performing database insertion..." << std::endl;

    fs::path db_dir = ResolveDatabaseDirectory(config);
    fs::create_directories(db_dir);

    std::vector<std::string> json_files =
        FileReader::FindFilesByExtension(config.log_filepath_, ".json");
    if (json_files.empty()) {
      std::cout << "Warning: No .json files found to insert." << std::endl;
      return AppExitCode::kSuccess;
    }

    int success_count = 0;
    for (const auto& json_path : json_files) {
      std::cout << "--- Inserting file: " << json_path << " ---" << std::endl;
      auto json_data_opt = JsonReader::ReadFile(json_path);
      if (json_data_opt.has_value()) {
        auto training_data =
            Serializer::Deserialize(json_data_opt.value().get());

        auto insert_result = repository_.InsertTrainingData(training_data);
        if (insert_result.IsSuccess()) {
          std::cout << "Successfully inserted data from " << json_path
                    << std::endl;
          success_count++;
        } else {
          std::cerr << insert_result.message_ << std::endl;
          std::cerr << "Failed to insert data from " << json_path << std::endl;
        }
      }
    }
    std::cout << "\nDatabase insertion complete. " << success_count << " of "
              << json_files.size() << " files inserted successfully."
              << std::endl;
    return (success_count == static_cast<int>(json_files.size()))
               ? AppExitCode::kSuccess
               : AppExitCode::kDatabaseError;
  }

  if (config.action_ == ActionType::Export) {
    std::cout << "Performing report export from database..." << std::endl;

    fs::path db_path = ResolveDatabasePath(config);
    if (!fs::exists(db_path)) {
      std::cerr << "Error: Database file not found at " << db_path.string()
                << std::endl;
      return AppExitCode::kFileNotFound;
    }

    fs::path output_dir = ResolveReportOutputDir(config);
    fs::create_directories(output_dir);

    auto export_result = report_exporter_.ExportReports(output_dir.string());
    if (export_result.IsSuccess()) {
      std::cout << "\nReport export completed successfully." << std::endl;
      return AppExitCode::kSuccess;
    }

    if (!export_result.message_.empty()) {
      std::cerr << export_result.message_ << std::endl;
    }
    std::cerr << "\nReport export failed." << std::endl;
    return ToExitCode(export_result);
  }

  if (config.action_ == ActionType::QueryPR ||
      config.action_ == ActionType::ListExercises ||
      config.action_ == ActionType::QueryCycles ||
      config.action_ == ActionType::QueryVolume) {
    fs::path db_dir = ResolveDatabaseDirectory(config);
    fs::create_directories(db_dir);
    fs::path db_path = ResolveDatabasePath(config);
    if (!fs::exists(db_path)) {
      std::cerr << "Error: Database file not found at " << db_path.string()
                << std::endl;
      return AppExitCode::kFileNotFound;
    }

    if (config.action_ == ActionType::QueryPR) {
      auto result = repository_.QueryPersonalRecords();
      if (!result.IsSuccess()) {
        return ToExitCode(result);
      }
      const auto& prs = result.payload_.value();
      if (prs.empty()) {
        std::cout << "No PR data found." << std::endl;
        return AppExitCode::kSuccess;
      }
      PrintPersonalRecords(prs);
      return AppExitCode::kSuccess;
    }

    if (config.action_ == ActionType::ListExercises) {
      auto result = repository_.ListExercises(config.type_filter_);
      if (!result.IsSuccess()) {
        return ToExitCode(result);
      }
      const auto& exercises = result.payload_.value();
      if (exercises.empty()) {
        std::cout << "No exercises found"
                  << (config.type_filter_.empty()
                          ? "."
                          : " for type: " + config.type_filter_)
                  << std::endl;
        return AppExitCode::kSuccess;
      }
      PrintExercises(exercises);
      return AppExitCode::kSuccess;
    }

    if (config.action_ == ActionType::QueryCycles) {
      auto result = repository_.ListCycles();
      if (!result.IsSuccess()) {
        return ToExitCode(result);
      }
      const auto& cycles = result.payload_.value();
      if (cycles.empty()) {
        std::cout << "No training cycles found." << std::endl;
        return AppExitCode::kSuccess;
      }
      PrintCycles(cycles);
      return AppExitCode::kSuccess;
    }

    if (config.action_ == ActionType::QueryVolume) {
      auto result = repository_.QueryVolumeStats(config.cycle_id_filter_,
                                                 config.type_filter_);
      if (!result.IsSuccess()) {
        return ToExitCode(result);
      }
      const auto& stats_opt = result.payload_.value();
      if (!stats_opt.has_value()) {
        std::cout << "No data found for Cycle: " << config.cycle_id_filter_
                  << ", Type: " << config.type_filter_ << std::endl;
        return AppExitCode::kSuccess;
      }
      PrintVolumeStats(stats_opt.value());
      return AppExitCode::kSuccess;
    }
  }

  return AppExitCode::kUnknownError;
}

auto DatabaseHandler::InsertData(const std::vector<DailyData>& data,
                                 const AppConfig& config) -> AppExitCode {
  std::cout << "Performing database insertion..." << std::endl;

  fs::path db_dir = ResolveDatabaseDirectory(config);
  fs::create_directories(db_dir);
  auto insert_result = repository_.InsertTrainingData(data);
  if (insert_result.IsSuccess()) {
    std::cout << "Successfully inserted data." << std::endl;
    return AppExitCode::kSuccess;
  }

  if (!insert_result.message_.empty()) {
    std::cerr << insert_result.message_ << std::endl;
  }
  std::cerr << "Failed to insert data." << std::endl;
  return ToExitCode(insert_result);
}
