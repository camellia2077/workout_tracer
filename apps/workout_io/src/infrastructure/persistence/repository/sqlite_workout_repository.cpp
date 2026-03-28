#include "infrastructure/persistence/repository/sqlite_workout_repository.hpp"

#include <utility>

#include "core/application/core_error_code.hpp"
#include "infrastructure/persistence/facade/db_facade.hpp"
#include "infrastructure/persistence/facade/query_facade.hpp"
#include "infrastructure/persistence/manager/db_manager.hpp"

namespace {

auto MapPersonalRecord(const PersonalRecord& record) -> WorkoutPersonalRecord {
  return {
      .exercise_name_ = record.exercise_name,
      .max_weight_ = record.max_weight,
      .reps_ = record.reps,
      .date_ = record.date,
      .estimated_1rm_epley_ = record.estimated_1rm_epley,
      .estimated_1rm_brzycki_ = record.estimated_1rm_brzycki,
  };
}

auto MapExerciseInfo(const ExerciseInfo& info) -> WorkoutExerciseInfo {
  return {
      .name_ = info.name,
      .type_ = info.type,
  };
}

auto MapCycleRecord(const CycleRecord& record) -> WorkoutCycleRecord {
  return {
      .cycle_id_ = record.cycle_id,
      .total_days_ = record.total_days,
      .type_ = record.type,
      .start_date_ = record.start_date,
      .end_date_ = record.end_date,
  };
}

auto MapVolumeStats(const VolumeStats& stats) -> WorkoutVolumeStats {
  return {
      .cycle_id_ = stats.cycle_id,
      .exercise_type_ = stats.exercise_type,
      .total_volume_ = stats.total_volume,
      .total_days_ = stats.total_days,
      .average_intensity_ = stats.average_intensity,
      .session_count_ = stats.session_count,
      .total_reps_ = stats.total_reps,
      .total_sets_ = stats.total_sets,
      .vol_power_ = stats.vol_power,
      .vol_hypertrophy_ = stats.vol_hypertrophy,
      .vol_endurance_ = stats.vol_endurance,
  };
}

}  // namespace

SqliteWorkoutRepository::SqliteWorkoutRepository(std::string db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteWorkoutRepository::InsertTrainingData(
    const std::vector<DailyData>& data) -> UseCaseResult<void> {
  DbManager db_manager(db_path_);
  if (!db_manager.Open()) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kDatabaseError,
                                        "failed to open database");
  }

  if (!DbFacade::InsertTrainingData(db_manager.GetConnection(), data)) {
    return UseCaseResult<void>::Failure(CoreErrorCode::kDatabaseError,
                                        "failed to insert training data");
  }

  return UseCaseResult<void>::Success();
}

auto SqliteWorkoutRepository::QueryPersonalRecords()
    -> UseCaseResult<std::vector<WorkoutPersonalRecord>> {
  DbManager db_manager(db_path_);
  if (!db_manager.Open()) {
    return UseCaseResult<std::vector<WorkoutPersonalRecord>>::Failure(
        CoreErrorCode::kDatabaseError, "failed to open database");
  }

  auto records = QueryFacade::QueryAllPRs(db_manager.GetConnection());
  std::vector<WorkoutPersonalRecord> mapped_records;
  mapped_records.reserve(records.size());
  for (const auto& record : records) {
    mapped_records.push_back(MapPersonalRecord(record));
  }

  return UseCaseResult<std::vector<WorkoutPersonalRecord>>::Success(
      std::move(mapped_records));
}

auto SqliteWorkoutRepository::ListExercises(const std::string& type_filter)
    -> UseCaseResult<std::vector<WorkoutExerciseInfo>> {
  DbManager db_manager(db_path_);
  if (!db_manager.Open()) {
    return UseCaseResult<std::vector<WorkoutExerciseInfo>>::Failure(
        CoreErrorCode::kDatabaseError, "failed to open database");
  }

  auto exercises =
      QueryFacade::GetExercisesByType(db_manager.GetConnection(), type_filter);
  std::vector<WorkoutExerciseInfo> mapped_exercises;
  mapped_exercises.reserve(exercises.size());
  for (const auto& exercise : exercises) {
    mapped_exercises.push_back(MapExerciseInfo(exercise));
  }

  return UseCaseResult<std::vector<WorkoutExerciseInfo>>::Success(
      std::move(mapped_exercises));
}

auto SqliteWorkoutRepository::ListCycles()
    -> UseCaseResult<std::vector<WorkoutCycleRecord>> {
  DbManager db_manager(db_path_);
  if (!db_manager.Open()) {
    return UseCaseResult<std::vector<WorkoutCycleRecord>>::Failure(
        CoreErrorCode::kDatabaseError, "failed to open database");
  }

  auto cycles = QueryFacade::GetAllCycles(db_manager.GetConnection());
  std::vector<WorkoutCycleRecord> mapped_cycles;
  mapped_cycles.reserve(cycles.size());
  for (const auto& cycle : cycles) {
    mapped_cycles.push_back(MapCycleRecord(cycle));
  }

  return UseCaseResult<std::vector<WorkoutCycleRecord>>::Success(
      std::move(mapped_cycles));
}

auto SqliteWorkoutRepository::QueryVolumeStats(const std::string& cycle_id,
                                               const std::string& type_filter)
    -> UseCaseResult<std::optional<WorkoutVolumeStats>> {
  DbManager db_manager(db_path_);
  if (!db_manager.Open()) {
    return UseCaseResult<std::optional<WorkoutVolumeStats>>::Failure(
        CoreErrorCode::kDatabaseError, "failed to open database");
  }

  auto stats_opt = QueryFacade::GetVolumeStats(db_manager.GetConnection(),
                                               cycle_id, type_filter);
  if (!stats_opt.has_value()) {
    return UseCaseResult<std::optional<WorkoutVolumeStats>>::Success(
        std::nullopt);
  }

  return UseCaseResult<std::optional<WorkoutVolumeStats>>::Success(
      MapVolumeStats(stats_opt.value()));
}
