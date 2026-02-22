#ifndef APPLICATION_INTERFACES_I_WORKOUT_REPOSITORY_HPP_
#define APPLICATION_INTERFACES_I_WORKOUT_REPOSITORY_HPP_

#include <optional>
#include <string>
#include <vector>

#include "application/interfaces/workout_query_models.hpp"
#include "domain/models/workout_item.hpp"

class IWorkoutRepository {
public:
  virtual ~IWorkoutRepository() = default;

  [[nodiscard]] virtual auto InsertTrainingData(
      const std::vector<DailyData>& data) -> bool = 0;

  [[nodiscard]] virtual auto QueryPersonalRecords()
      -> std::vector<WorkoutPersonalRecord> = 0;

  [[nodiscard]] virtual auto ListExercises(const std::string& type_filter)
      -> std::vector<WorkoutExerciseInfo> = 0;

  [[nodiscard]] virtual auto ListCycles() -> std::vector<WorkoutCycleRecord> = 0;

  [[nodiscard]] virtual auto QueryVolumeStats(const std::string& cycle_id,
                                              const std::string& type_filter)
      -> std::optional<WorkoutVolumeStats> = 0;
};

#endif  // APPLICATION_INTERFACES_I_WORKOUT_REPOSITORY_HPP_
