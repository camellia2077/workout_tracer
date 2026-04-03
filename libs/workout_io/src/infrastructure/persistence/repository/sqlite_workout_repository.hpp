#ifndef INFRASTRUCTURE_PERSISTENCE_REPOSITORY_SQLITE_WORKOUT_REPOSITORY_HPP_
#define INFRASTRUCTURE_PERSISTENCE_REPOSITORY_SQLITE_WORKOUT_REPOSITORY_HPP_

#include <string>
#include <vector>

#include "application/interfaces/i_workout_repository.hpp"

class SqliteWorkoutRepository : public IWorkoutRepository {
public:
  explicit SqliteWorkoutRepository(std::string db_path);

  [[nodiscard]] auto InsertTrainingData(
      const std::vector<DailyData>& data) -> UseCaseResult<void> override;

  [[nodiscard]] auto QueryPersonalRecords()
      -> UseCaseResult<std::vector<WorkoutPersonalRecord>> override;

  [[nodiscard]] auto ListExercises(const std::string& type_filter)
      -> UseCaseResult<std::vector<WorkoutExerciseInfo>> override;

  [[nodiscard]] auto ListCycles()
      -> UseCaseResult<std::vector<WorkoutCycleRecord>> override;

  [[nodiscard]] auto QueryVolumeStats(const std::string& cycle_id,
                                      const std::string& type_filter)
      -> UseCaseResult<std::optional<WorkoutVolumeStats>> override;

private:
  std::string db_path_;
};

#endif  // INFRASTRUCTURE_PERSISTENCE_REPOSITORY_SQLITE_WORKOUT_REPOSITORY_HPP_
