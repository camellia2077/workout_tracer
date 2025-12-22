# cmake/SourceFileCollection.cmake

# --- Common 模块 ---
set(COMMON_SOURCES
    src/common/JsonReader.cpp
    src/common/FileReader.cpp
)

# --- CLI 模块 ---
set(CLI_SOURCES
    src/cli/CommandLineParser.cpp
)

# --- Reprocessor 模块 ---
set(REPROCESSOR_SOURCES
    src/reprocessor/facade/Reprocessor.cpp
    src/reprocessor/preprocessor/Preprocessor.cpp
    src/reprocessor/preprocessor/date_processor/DateProcessor.cpp
    src/reprocessor/preprocessor/log_formatter/JsonFormatter.cpp
    src/reprocessor/preprocessor/log_parser/LogParser.cpp
    src/reprocessor/preprocessor/name_mapper/ProjectNameMapper.cpp
    src/reprocessor/preprocessor/volume_calculator/VolumeCalculator.cpp
    src/reprocessor/validator/Validator.cpp
    src/reprocessor/validator/LineValidator.cpp
)

# --- Controller 模块 ---
set(CONTROLLER_SOURCES
    src/controller/ActionHandler.cpp
    src/controller/FileProcessorHandler.cpp
    src/controller/DatabaseHandler.cpp
)

# --- DB 模块 ---
set(DB_SOURCES
    src/db/facade/DbFacade.cpp
    src/db/manager/DbManager.cpp
    src/db/inserter/DataInserter.cpp
)

# --- Report 模块 ---
set(REPORT_SOURCES
    src/report/database/DatabaseManager.cpp
    src/report/formatter/MarkdownFormatter.cpp
    src/report/facade/ReportFacade.cpp
)

# --- 汇总 ---
set(ALL_SOURCES
    ${COMMON_SOURCES}
    ${CLI_SOURCES}
    ${REPROCESSOR_SOURCES}
    ${CONTROLLER_SOURCES}
    ${DB_SOURCES}
    ${REPORT_SOURCES}
)