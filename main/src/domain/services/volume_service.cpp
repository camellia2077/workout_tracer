// domain/services/volume_service.cpp
#include "domain/services/volume_service.hpp"

#include <numeric>

auto VolumeService::CalculateProjectVolume(std::vector<SetData>& sets)
    -> double {
  double total_volume = 0.0;

  for (auto& set_item : sets) {
    // If weight is negative (e.g., resistance band assistance), do not count
    // volume
    if (set_item.weight_ < 0) {
      set_item.volume_ = 0.0;
      continue;
    }

    set_item.volume_ = set_item.weight_ * set_item.reps_;
    total_volume += set_item.volume_;
  }

  return total_volume;
}

auto VolumeService::CalculateVolume(std::vector<DailyData>& all_data) -> void {
  for (auto& daily_data : all_data) {
    for (auto& project : daily_data.projects_) {
      // Delegate to helper for calculation
      project.total_volume_ = CalculateProjectVolume(project.sets_);
    }
  }
}
