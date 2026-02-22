// application/database_handler.hpp

#ifndef APPLICATION_DATABASE_HANDLER_HPP_
#define APPLICATION_DATABASE_HANDLER_HPP_
#include "application/action_handler.hpp"
#include "domain/models/workout_item.hpp"
#include <vector>

// 这个类专门处理与数据库相关的所有操作。
class DatabaseHandler {
public:
  [[nodiscard]] static auto Handle(const AppConfig& config) -> AppExitCode;
  [[nodiscard]] static auto InsertData(const std::vector<DailyData>& data,
                                       const AppConfig& config) -> AppExitCode;
};

#endif // APPLICATION_DATABASE_HANDLER_HPP_