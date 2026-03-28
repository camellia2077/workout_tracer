// infrastructure/persistence/facade/query_facade.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_FACADE_QUERY_FACADE_HPP_
#define INFRASTRUCTURE_PERSISTENCE_FACADE_QUERY_FACADE_HPP_

#include "sqlite3.h"
#include <optional>
#include <string>
#include <vector>

struct PersonalRecord {
  std::string exercise_name;
  double max_weight;
  int reps;
  std::string date;
  double estimated_1rm_epley;
  double estimated_1rm_brzycki;
};

struct ExerciseInfo {
  std::string name;
  std::string type;
};

struct CycleRecord {
  std::string cycle_id;
  int total_days;
  std::string type;
  std::string start_date;
  std::string end_date;
};

struct VolumeStats {
  std::string cycle_id;
  std::string exercise_type;
  double total_volume;
  int total_days;
  double average_intensity;
  int session_count;
  int total_reps;
  int total_sets;
  double vol_power;       // 1-5 reps
  double vol_hypertrophy; // 6-12 reps
  double vol_endurance;   // 13+ reps
};

class QueryFacade {
public:
  static auto QueryAllPRs(sqlite3* db) -> std::vector<PersonalRecord>;
  static auto GetExercisesByType(sqlite3* db, const std::string& type_filter) 
      -> std::vector<ExerciseInfo>;
  static auto GetAllCycles(sqlite3* db) -> std::vector<CycleRecord>;
  static auto GetVolumeStats(sqlite3* db, const std::string& cycle_id, const std::string& type)
      -> std::optional<VolumeStats>;
};

#endif // INFRASTRUCTURE_PERSISTENCE_FACADE_QUERY_FACADE_HPP_
