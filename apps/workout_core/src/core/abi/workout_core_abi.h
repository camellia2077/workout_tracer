#ifndef CORE_ABI_WORKOUT_CORE_ABI_H_
#define CORE_ABI_WORKOUT_CORE_ABI_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum workout_core_status_code_t {
  WORKOUT_CORE_STATUS_SUCCESS = 0,
  WORKOUT_CORE_STATUS_INVALID_ARGS = 1,
  WORKOUT_CORE_STATUS_VALIDATION_ERROR = 2,
  WORKOUT_CORE_STATUS_FILE_NOT_FOUND = 3,
  WORKOUT_CORE_STATUS_DATABASE_ERROR = 4,
  WORKOUT_CORE_STATUS_PROCESSING_ERROR = 5,
  WORKOUT_CORE_STATUS_UNKNOWN_ERROR = 99,
  WORKOUT_CORE_STATUS_NOT_IMPLEMENTED = 100
} workout_core_status_code_t;

typedef enum workout_core_command_t {
  WORKOUT_CORE_COMMAND_UNKNOWN = 0,
  WORKOUT_CORE_COMMAND_VALIDATE = 1,
  WORKOUT_CORE_COMMAND_CONVERT = 2,
  WORKOUT_CORE_COMMAND_INSERT = 3,
  WORKOUT_CORE_COMMAND_EXPORT = 4,
  WORKOUT_CORE_COMMAND_INGEST = 5,
  WORKOUT_CORE_COMMAND_QUERY_PR = 6,
  WORKOUT_CORE_COMMAND_LIST_EXERCISES = 7,
  WORKOUT_CORE_COMMAND_QUERY_CYCLES = 8,
  WORKOUT_CORE_COMMAND_QUERY_VOLUME = 9
} workout_core_command_t;

typedef struct workout_core_request_t {
  workout_core_command_t command;
  const char* request_json_utf8;
} workout_core_request_t;

typedef struct workout_core_result_t {
  workout_core_status_code_t status_code;
  char* message_utf8;
  char* payload_json_utf8;
} workout_core_result_t;

typedef workout_core_result_t (*workout_core_execute_delegate_t)(
    const workout_core_request_t* request);

// Creates a result with heap-owned strings. Call workout_core_result_dispose
// after use.
workout_core_result_t workout_core_make_result(
    workout_core_status_code_t status_code, const char* message_utf8,
    const char* payload_json_utf8);

// Returns a placeholder result until concrete C ABI use-cases are wired.
workout_core_result_t workout_core_not_implemented_result(void);

// Registers execution delegate used by workout_core_execute_request.
void workout_core_set_execute_delegate(
    workout_core_execute_delegate_t delegate);

// Executes a request through C ABI dispatcher.
workout_core_result_t workout_core_execute_request(
    const workout_core_request_t* request);

// Frees memory owned by result and resets fields to null.
void workout_core_result_dispose(workout_core_result_t* result);

#ifdef __cplusplus
}
#endif

#endif  // CORE_ABI_WORKOUT_CORE_ABI_H_
