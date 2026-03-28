#include "core/abi/workout_core_abi.h"

#include <cstdlib>
#include <cstring>

namespace {
auto kEmptyJsonPayload = "{}";

workout_core_execute_delegate_t g_execute_delegate = nullptr;

auto DuplicateCString(const char* source) -> char* {
  if (source == nullptr) {
    return nullptr;
  }

  const std::size_t length = std::strlen(source);
  auto* copy = static_cast<char*>(std::malloc(length + 1));
  if (copy == nullptr) {
    return nullptr;
  }

  std::memcpy(copy, source, length + 1);
  return copy;
}
}  // namespace

auto workout_core_make_result(workout_core_status_code_t status_code,
                              const char* message_utf8,
                              const char* payload_json_utf8)
    -> workout_core_result_t {
  workout_core_result_t result{};
  result.status_code = status_code;
  result.message_utf8 =
      DuplicateCString(message_utf8 == nullptr ? "" : message_utf8);
  result.payload_json_utf8 = DuplicateCString(
      payload_json_utf8 == nullptr ? kEmptyJsonPayload : payload_json_utf8);
  return result;
}

auto workout_core_not_implemented_result(void) -> workout_core_result_t {
  return workout_core_make_result(WORKOUT_CORE_STATUS_NOT_IMPLEMENTED,
                                  "C ABI use-case is not wired yet.",
                                  kEmptyJsonPayload);
}

auto workout_core_set_execute_delegate(workout_core_execute_delegate_t delegate)
    -> void {
  g_execute_delegate = delegate;
}

auto workout_core_execute_request(const workout_core_request_t* request)
    -> workout_core_result_t {
  if (request == nullptr) {
    return workout_core_make_result(WORKOUT_CORE_STATUS_INVALID_ARGS,
                                    "request must not be null",
                                    kEmptyJsonPayload);
  }

  if (g_execute_delegate == nullptr) {
    return workout_core_not_implemented_result();
  }

  return g_execute_delegate(request);
}

auto workout_core_result_dispose(workout_core_result_t* result) -> void {
  if (result == nullptr) {
    return;
  }

  if (result->message_utf8 != nullptr) {
    std::free(result->message_utf8);
    result->message_utf8 = nullptr;
  }

  if (result->payload_json_utf8 != nullptr) {
    std::free(result->payload_json_utf8);
    result->payload_json_utf8 = nullptr;
  }

  result->status_code = WORKOUT_CORE_STATUS_SUCCESS;
}
