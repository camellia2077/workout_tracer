# cmake/source_file_collection.cmake

# --- Common 模块 ---
set(COMMON_SOURCES
    src/common/json_reader.cpp
    src/common/file_reader.cpp
)

# --- CLI 模块 ---
set(CLI_SOURCES
    src/cli/framework/application.cpp
)

# --- Validator 模块 ---
set(VALIDATOR_SOURCES
    src/infrastructure/validation/validator.cpp
    src/infrastructure/validation/internal/line_validator.cpp
)

# --- Converter 模块 ---
set(CONVERTER_SOURCES
    src/infrastructure/converter/converter.cpp
    src/infrastructure/converter/log_parser.cpp
    src/infrastructure/converter/project_name_mapper.cpp
)

# --- Domain Layer ---
set(DOMAIN_SERVICES_SOURCES
    src/domain/services/volume_service.cpp
    src/domain/services/date_service.cpp
)

# --- Serializer 模块 ---
set(SERIALIZER_SOURCES
    src/infrastructure/serializer/serializer.cpp
)

# --- Controller 模块 ---
set(CONTROLLER_SOURCES
    src/application/action_handler.cpp
    src/application/file_processor_handler.cpp
    src/application/database_handler.cpp
)

# --- DB 模块 ---
set(DB_SOURCES
    src/infrastructure/persistence/facade/db_facade.cpp
    src/infrastructure/persistence/facade/query_facade.cpp
    src/infrastructure/persistence/manager/db_manager.cpp
    src/infrastructure/persistence/inserter/data_inserter.cpp
)

# --- Report 模块 ---
set(REPORT_SOURCES
    src/infrastructure/reporting/database/database_manager.cpp
    src/infrastructure/reporting/formatter/markdown_formatter.cpp
    src/infrastructure/reporting/facade/report_facade.cpp
)

# --- 汇总 ---
set(ALL_SOURCES
    ${COMMON_SOURCES}
    ${CLI_SOURCES}
    ${VALIDATOR_SOURCES}
    ${CONVERTER_SOURCES}
    ${DOMAIN_SERVICES_SOURCES}
    ${SERIALIZER_SOURCES}
    ${CONTROLLER_SOURCES}
    ${DB_SOURCES}
    ${REPORT_SOURCES}
)