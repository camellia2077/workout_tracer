# cmake/source_file_collection.cmake

# --- CLI 模块 ---
set(CLI_SOURCES
    workout_windows_cli/src/cli/framework/application.cpp
    workout_windows_cli/src/cli/framework/core_request_mapper.cpp
    workout_windows_cli/src/cli/framework/core_status_mapper.cpp
)

# --- Workout IO: Common ---
set(WORKOUT_IO_COMMON_SOURCES
    workout_io/src/common/json_reader.cpp
    workout_io/src/common/file_reader.cpp
)

# --- Workout IO: Validation ---
set(WORKOUT_IO_VALIDATION_SOURCES
    workout_io/src/infrastructure/validation/validator.cpp
    workout_io/src/infrastructure/validation/internal/line_validator.cpp
)

# --- Workout IO: Converter ---
set(WORKOUT_IO_CONVERTER_SOURCES
    workout_io/src/infrastructure/converter/converter.cpp
    workout_io/src/infrastructure/converter/log_parser.cpp
    workout_io/src/infrastructure/converter/project_name_mapper.cpp
)

# --- Workout IO: Serializer ---
set(WORKOUT_IO_SERIALIZER_SOURCES
    workout_io/src/infrastructure/serializer/serializer.cpp
)

# --- Workout IO: Persistence ---
set(WORKOUT_IO_PERSISTENCE_SOURCES
    workout_io/src/infrastructure/persistence/facade/db_facade.cpp
    workout_io/src/infrastructure/persistence/facade/query_facade.cpp
    workout_io/src/infrastructure/persistence/manager/db_manager.cpp
    workout_io/src/infrastructure/persistence/inserter/data_inserter.cpp
    workout_io/src/infrastructure/persistence/repository/sqlite_workout_repository.cpp
)

# --- Workout IO: Reporting ---
set(WORKOUT_IO_REPORTING_SOURCES
    workout_io/src/infrastructure/reporting/database/database_manager.cpp
    workout_io/src/infrastructure/reporting/exporter/sqlite_report_exporter.cpp
    workout_io/src/infrastructure/reporting/formatter/markdown_formatter.cpp
    workout_io/src/infrastructure/reporting/facade/report_facade.cpp
)

# --- Workout IO 汇总 ---
set(WORKOUT_IO_SOURCES
    ${WORKOUT_IO_COMMON_SOURCES}
    ${WORKOUT_IO_VALIDATION_SOURCES}
    ${WORKOUT_IO_CONVERTER_SOURCES}
    ${WORKOUT_IO_SERIALIZER_SOURCES}
    ${WORKOUT_IO_PERSISTENCE_SOURCES}
    ${WORKOUT_IO_REPORTING_SOURCES}
)

# --- Workout Core: Domain ---
set(CORE_DOMAIN_SOURCES
    workout_core/src/domain/services/volume_service.cpp
    workout_core/src/domain/services/date_service.cpp
    workout_core/src/domain/services/training_metrics_service.cpp
)

# --- Workout Core: ABI ---
set(CORE_ABI_SOURCES
    workout_core/src/core/abi/workout_core_abi.cpp
)

# --- Workout Core: Application ---
set(CORE_APPLICATION_SOURCES
    workout_core/src/application/action_handler.cpp
    workout_core/src/application/action_handler_abi_bridge.cpp
    workout_core/src/application/file_processor_handler.cpp
    workout_core/src/application/database_handler.cpp
)

# --- Workout Core 汇总 ---
set(CORE_SOURCES
    ${CORE_DOMAIN_SOURCES}
    ${CORE_ABI_SOURCES}
)

# --- 汇总 ---
set(ALL_SOURCES
    ${CLI_SOURCES}
    ${WORKOUT_IO_SOURCES}
    ${CORE_SOURCES}
    ${CORE_APPLICATION_SOURCES}
)
