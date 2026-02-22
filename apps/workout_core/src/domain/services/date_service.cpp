// domain/services/date_service.cpp
#include "domain/services/date_service.hpp"

auto DateService::CompleteDates(std::vector<DailyData>& all_data,
                                int year_to_use) -> void {
  for (auto& daily : all_data) {
    if (daily.date_.length() == 4) {  // Only process "MMDD" format
      std::string month = daily.date_.substr(0, 2);
      std::string day = daily.date_.substr(2, 2);
      daily.date_ = std::to_string(year_to_use) + "-" + month + "-" + day;
    }
  }
}
