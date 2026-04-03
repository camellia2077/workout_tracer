#include "domain/services/training_metrics_service.hpp"

auto TrainingMetricsService::EstimateOneRmEpley(double weight, int reps)
    -> double {
  if (reps <= 1) {
    return weight;
  }
  return weight * (1.0 + static_cast<double>(reps) / 30.0);
}

auto TrainingMetricsService::EstimateOneRmBrzycki(double weight, int reps)
    -> double {
  if (reps <= 1) {
    return weight;
  }
  if (reps >= 37) {
    return weight * 36.0;
  }
  return weight * (36.0 / (37.0 - static_cast<double>(reps)));
}

auto TrainingMetricsService::AverageDailyVolume(double total_volume,
                                                int total_days) -> double {
  if (total_days <= 0) {
    return 0.0;
  }
  return total_volume / static_cast<double>(total_days);
}

auto TrainingMetricsService::SessionsPerWeek(int session_count, int total_days)
    -> double {
  if (total_days <= 0) {
    return 0.0;
  }
  return static_cast<double>(session_count) /
         (static_cast<double>(total_days) / 7.0);
}

auto TrainingMetricsService::SetsPerDay(int total_sets, int total_days)
    -> double {
  if (total_days <= 0) {
    return 0.0;
  }
  return static_cast<double>(total_sets) / static_cast<double>(total_days);
}

auto TrainingMetricsService::PercentageOfTotal(double part, double total)
    -> double {
  if (total <= 0.0) {
    return 0.0;
  }
  return part / total * 100.0;
}
