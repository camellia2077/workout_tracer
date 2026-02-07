// application/exit_code.hpp
#ifndef APPLICATION_EXIT_CODE_HPP_
#define APPLICATION_EXIT_CODE_HPP_

enum class AppExitCode : int {
  kSuccess = 0,
  kInvalidArgs = 1,
  kValidationError = 2,
  kFileNotFound = 3,
  kDatabaseError = 4,
  kProcessingError = 5,
  kUnknownError = 99
};

#endif // APPLICATION_EXIT_CODE_HPP_
