#ifndef DOMAIN_SERVICES_TRAINING_METRICS_SERVICE_HPP_
#define DOMAIN_SERVICES_TRAINING_METRICS_SERVICE_HPP_

class TrainingMetricsService {
public:
  [[nodiscard]] static auto EstimateOneRmEpley(double weight, int reps)
      -> double;
  [[nodiscard]] static auto EstimateOneRmBrzycki(double weight, int reps)
      -> double;

  [[nodiscard]] static auto AverageDailyVolume(double total_volume,
                                               int total_days) -> double;
  [[nodiscard]] static auto SessionsPerWeek(int session_count, int total_days)
      -> double;
  [[nodiscard]] static auto SetsPerDay(int total_sets, int total_days) -> double;
  [[nodiscard]] static auto PercentageOfTotal(double part, double total)
      -> double;
};

#endif  // DOMAIN_SERVICES_TRAINING_METRICS_SERVICE_HPP_
