#ifndef APPLICATION_INTERFACES_WORKOUT_QUERY_MODELS_HPP_
#define APPLICATION_INTERFACES_WORKOUT_QUERY_MODELS_HPP_

#include <optional>
#include <string>

struct WorkoutPersonalRecord {
  std::string exercise_name_;
  double max_weight_kg_ = 0.0;
  std::string original_unit_ = "kg";
  double original_weight_value_ = 0.0;
  int reps_ = 0;
  std::string date_;
  double estimated_1rm_epley_ = 0.0;
  double estimated_1rm_brzycki_ = 0.0;
};

struct WorkoutExerciseInfo {
  std::string name_;
  std::string type_;
};

struct WorkoutCycleRecord {
  std::string cycle_id_;
  int total_days_ = 0;
  std::string type_;
  std::string start_date_;
  std::string end_date_;
};

struct WorkoutVolumeStats {
  std::string cycle_id_;
  std::string exercise_type_;
  double total_volume_ = 0.0;
  std::string common_original_unit_;
  int total_days_ = 0;
  double average_intensity_ = 0.0;
  int session_count_ = 0;
  int total_reps_ = 0;
  int total_sets_ = 0;
  double vol_power_ = 0.0;
  double vol_hypertrophy_ = 0.0;
  double vol_endurance_ = 0.0;
};

#endif  // APPLICATION_INTERFACES_WORKOUT_QUERY_MODELS_HPP_
