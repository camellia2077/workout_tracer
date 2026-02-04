// domain/services/volume_service.hpp
#ifndef DOMAIN_SERVICES_VOLUME_SERVICE_HPP_
#define DOMAIN_SERVICES_VOLUME_SERVICE_HPP_

#include "domain/models/workout_item.hpp"
#include <vector>

// Domain Service for calculating training volume
class VolumeService {
public:
  // Calculate volume for all items
  static auto CalculateVolume(std::vector<DailyData>& all_data) -> void;

private:
  // Helper: Calculate volume for a single project
  static auto CalculateProjectVolume(std::vector<SetData>& sets) -> double;
};

#endif // DOMAIN_SERVICES_VOLUME_SERVICE_HPP_
