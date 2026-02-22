#include "cli/framework/core_status_mapper.hpp"

namespace cli::framework {

auto MapCoreStatusToExitCode(workout_core_status_code_t status) -> AppExitCode {
  switch (status) {
    case WORKOUT_CORE_STATUS_SUCCESS:
      return AppExitCode::kSuccess;
    case WORKOUT_CORE_STATUS_INVALID_ARGS:
      return AppExitCode::kInvalidArgs;
    case WORKOUT_CORE_STATUS_VALIDATION_ERROR:
      return AppExitCode::kValidationError;
    case WORKOUT_CORE_STATUS_FILE_NOT_FOUND:
      return AppExitCode::kFileNotFound;
    case WORKOUT_CORE_STATUS_DATABASE_ERROR:
      return AppExitCode::kDatabaseError;
    case WORKOUT_CORE_STATUS_PROCESSING_ERROR:
      return AppExitCode::kProcessingError;
    case WORKOUT_CORE_STATUS_NOT_IMPLEMENTED:
      return AppExitCode::kProcessingError;
    case WORKOUT_CORE_STATUS_UNKNOWN_ERROR:
    default:
      return AppExitCode::kUnknownError;
  }
}

}  // namespace cli::framework
