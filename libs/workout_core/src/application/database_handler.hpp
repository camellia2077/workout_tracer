// application/database_handler.hpp

#ifndef APPLICATION_DATABASE_HANDLER_HPP_
#define APPLICATION_DATABASE_HANDLER_HPP_

#include <vector>

#include "application/action_handler.hpp"
#include "application/interfaces/i_report_exporter.hpp"
#include "application/interfaces/i_workout_repository.hpp"
#include "domain/models/workout_item.hpp"

// 这个类专门处理与数据库相关的所有操作。
class DatabaseHandler {
public:
  DatabaseHandler(IWorkoutRepository& repository, IReportExporter& report_exporter);

  [[nodiscard]] auto Handle(const AppConfig& config) -> AppExitCode;
  [[nodiscard]] auto InsertData(const std::vector<DailyData>& data,
                                const AppConfig& config) -> AppExitCode;

private:
  IWorkoutRepository& repository_;
  IReportExporter& report_exporter_;
};

#endif // APPLICATION_DATABASE_HANDLER_HPP_
