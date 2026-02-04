// domain/services/date_service.hpp
#ifndef DOMAIN_SERVICES_DATE_SERVICE_HPP_
#define DOMAIN_SERVICES_DATE_SERVICE_HPP_

#include "domain/models/workout_item.hpp"
#include <vector>

// Domain Service for date processing
class DateService {
public:
  /**
   * @brief Complete the year for data and format as YYYY-MM-DD.
   * @param all_data Reference to data to process.
   * @param year_to_use The 4-digit year to apply.
   */
  static auto CompleteDates(std::vector<DailyData>& all_data, int year_to_use) -> void;
};

#endif // DOMAIN_SERVICES_DATE_SERVICE_HPP_
