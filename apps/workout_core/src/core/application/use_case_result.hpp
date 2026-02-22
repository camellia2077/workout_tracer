#ifndef CORE_APPLICATION_USE_CASE_RESULT_HPP_
#define CORE_APPLICATION_USE_CASE_RESULT_HPP_

#include <optional>
#include <string>
#include <utility>

#include "core/application/core_error_code.hpp"

template <typename PayloadType>
struct UseCaseResult {
  CoreErrorCode error_code_ = CoreErrorCode::kUnknownError;
  std::string message_;
  std::optional<PayloadType> payload_;

  [[nodiscard]] static auto Success(PayloadType payload) -> UseCaseResult {
    return {
        .error_code_ = CoreErrorCode::kSuccess,
        .message_ = "",
        .payload_ = std::move(payload),
    };
  }

  [[nodiscard]] static auto Failure(CoreErrorCode error_code,
                                    std::string message) -> UseCaseResult {
    return {
        .error_code_ = error_code,
        .message_ = std::move(message),
        .payload_ = std::nullopt,
    };
  }

  [[nodiscard]] auto IsSuccess() const -> bool {
    return error_code_ == CoreErrorCode::kSuccess;
  }
};

template <>
struct UseCaseResult<void> {
  CoreErrorCode error_code_ = CoreErrorCode::kUnknownError;
  std::string message_;

  [[nodiscard]] static auto Success() -> UseCaseResult {
    return {
        .error_code_ = CoreErrorCode::kSuccess,
        .message_ = "",
    };
  }

  [[nodiscard]] static auto Failure(CoreErrorCode error_code,
                                    std::string message) -> UseCaseResult {
    return {
        .error_code_ = error_code,
        .message_ = std::move(message),
    };
  }

  [[nodiscard]] auto IsSuccess() const -> bool {
    return error_code_ == CoreErrorCode::kSuccess;
  }
};

#endif  // CORE_APPLICATION_USE_CASE_RESULT_HPP_
