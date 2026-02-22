#ifndef CLI_FRAMEWORK_CORE_REQUEST_MAPPER_HPP_
#define CLI_FRAMEWORK_CORE_REQUEST_MAPPER_HPP_

#include <string>

#include "application/action_handler.hpp"
#include "core/abi/workout_core_abi.h"

namespace cli::framework {

struct CoreRequestEnvelope {
  workout_core_command_t command_ = WORKOUT_CORE_COMMAND_UNKNOWN;
  std::string request_json_;

  [[nodiscard]] auto ToCRequest() const -> workout_core_request_t {
    workout_core_request_t request{};
    request.command = command_;
    request.request_json_utf8 = request_json_.c_str();
    return request;
  }
};

[[nodiscard]] auto BuildCoreRequestFromConfig(const AppConfig& config)
    -> CoreRequestEnvelope;

}  // namespace cli::framework

#endif  // CLI_FRAMEWORK_CORE_REQUEST_MAPPER_HPP_
