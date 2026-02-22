#ifndef CLI_FRAMEWORK_CORE_STATUS_MAPPER_HPP_
#define CLI_FRAMEWORK_CORE_STATUS_MAPPER_HPP_

#include "application/exit_code.hpp"
#include "core/abi/workout_core_abi.h"

namespace cli::framework {

[[nodiscard]] auto MapCoreStatusToExitCode(workout_core_status_code_t status)
    -> AppExitCode;

}  // namespace cli::framework

#endif  // CLI_FRAMEWORK_CORE_STATUS_MAPPER_HPP_
