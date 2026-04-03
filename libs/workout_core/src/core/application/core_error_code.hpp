#ifndef CORE_APPLICATION_CORE_ERROR_CODE_HPP_
#define CORE_APPLICATION_CORE_ERROR_CODE_HPP_

#include <cstdint>

enum class CoreErrorCode : std::uint32_t {
  kSuccess = 0,
  kInvalidArgs = 1,
  kValidationError = 2,
  kFileNotFound = 3,
  kDatabaseError = 4,
  kProcessingError = 5,
  kUnknownError = 99,
  kNotImplemented = 100
};

#endif  // CORE_APPLICATION_CORE_ERROR_CODE_HPP_
