# cmake/source_file_collection.cmake

# --- Common 模块 ---
set(COMMON_SOURCES
    workout_core/src/common/json_reader.cpp
    workout_core/src/common/file_reader.cpp
)

# --- CLI 模块 ---
set(CLI_SOURCES
    workout_windows_cli/src/cli/framework/application.cpp
    workout_windows_cli/src/cli/framework/core_request_mapper.cpp
    workout_windows_cli/src/cli/framework/core_status_mapper.cpp
)

# --- Validator 模块 ---
set(VALIDATOR_SOURCES
    workout_core/src/infrastructure/validation/validator.cpp
    workout_core/src/infrastructure/validation/internal/line_validator.cpp
)

# --- Converter 模块 ---
set(CONVERTER_SOURCES
    workout_core/src/infrastructure/converter/converter.cpp
    workout_core/src/infrastructure/converter/log_parser.cpp
    workout_core/src/infrastructure/converter/project_name_mapper.cpp
)

# --- Domain Layer ---
set(CORE_DOMAIN_SOURCES
    workout_core/src/domain/services/volume_service.cpp
    workout_core/src/domain/services/date_service.cpp
    workout_core/src/domain/services/training_metrics_service.cpp
)

# --- Core Application Layer (header-only for now) ---
set(CORE_APPLICATION_SOURCES
    workout_core/src/core/abi/workout_core_abi.cpp
)

# --- Core 汇总 ---
set(CORE_SOURCES
    ${CORE_DOMAIN_SOURCES}
    ${CORE_APPLICATION_SOURCES}
)

# --- Serializer 模块 ---
set(SERIALIZER_SOURCES
    workout_core/src/infrastructure/serializer/serializer.cpp
)

# --- Controller 模块 ---
set(CONTROLLER_SOURCES
    workout_core/src/application/action_handler.cpp
    workout_core/src/application/action_handler_abi_bridge.cpp
    workout_core/src/application/file_processor_handler.cpp
    workout_core/src/application/database_handler.cpp
)

# --- DB 模块 ---
set(DB_SOURCES
    workout_core/src/infrastructure/persistence/facade/db_facade.cpp
    workout_core/src/infrastructure/persistence/facade/query_facade.cpp
    workout_core/src/infrastructure/persistence/manager/db_manager.cpp
    workout_core/src/infrastructure/persistence/inserter/data_inserter.cpp
)

# --- Report 模块 ---
set(REPORT_SOURCES
    workout_core/src/infrastructure/reporting/database/database_manager.cpp
    workout_core/src/infrastructure/reporting/formatter/markdown_formatter.cpp
    workout_core/src/infrastructure/reporting/facade/report_facade.cpp
)

# --- 汇总 ---
set(ALL_SOURCES
    ${COMMON_SOURCES}
    ${CLI_SOURCES}
    ${VALIDATOR_SOURCES}
    ${CONVERTER_SOURCES}
    ${SERIALIZER_SOURCES}
    ${CONTROLLER_SOURCES}
    ${DB_SOURCES}
    ${REPORT_SOURCES}
)
