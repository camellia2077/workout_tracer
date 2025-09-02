# CMakeLists.txt

# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.16)

# 定义项目名称、版本和语言
project(WorkoutParser VERSION 1.0 LANGUAGES CXX)

# 设置C++标准为 C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 强制开启颜色输出 ---
set(CMAKE_COLOR_MAKEFILE ON)
if(NOT MSVC)
    set(CMAKE_CXX_DIAGNOSTICS_COLOR ALWAYS)
endif()

# --- 自动启用 ccache (如果可用) ---
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "ccache found, enabling for C/C++ compilers.")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_FOUND}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_FOUND}")
else()
    message(STATUS "ccache not found. For faster builds, consider installing it.")
endif()

# --- 为Release模式添加优化选项 ---
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")

# --- 查找整个项目都需要的库 ---
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)

# --- 定义配置文件的路径 ---
set(CONFIG_FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/config/mapping.json)

# ===================================================================
# 源文件列表 (按模块组织)
# ===================================================================

# --- Common 模块 ---
set(COMMON_SOURCES
    src/common/JsonReader.cpp
    src/common/FileReader.cpp
)

# --- Reprocessor 模块 ---
set(REPROCESSOR_SOURCES
    src/reprocessor/date_processor/DateProcessor.cpp
    
    src/reprocessor/log_formatter/JsonFormatter.cpp

    src/reprocessor/log_parser/LogParser.cpp
    src/reprocessor/name_mapper/ProjectNameMapper.cpp
    src/reprocessor/validator/Validator.cpp
    src/reprocessor/volume_calculator/VolumeCalculator.cpp
    src/reprocessor/Reprocessor.cpp
)

# --- Controller 模块 ---
set(CONTROLLER_SOURCES
    src/controller/ActionHandler.cpp
    src/controller/FileProcessorHandler.cpp
    src/controller/DatabaseHandler.cpp
)

# --- DB 模块 ---
set(DB_SOURCES
    src/db/DbInsertor.cpp
    src/db/DbManager.cpp
)

# --- Report 模块 ---
# [FIXED] 修正了这里的源文件路径
set(REPORT_SOURCES
    src/report/database/DatabaseManager.cpp
    src/report/formatter/MarkdownFormatter.cpp
    src/report/facade/ReportFacade.cpp
)

# --- 整合所有共享源文件 ---
set(SHARED_SOURCES
    ${COMMON_SOURCES}
    ${REPROCESSOR_SOURCES}
    ${CONTROLLER_SOURCES}
    ${DB_SOURCES}
    ${REPORT_SOURCES}
)

# ===================================================================
# 封装公共配置的函数
# ===================================================================
function(configure_workout_target TARGET_NAME ENTRY_POINT_FILE)
    add_executable(
        ${TARGET_NAME}
        ${ENTRY_POINT_FILE}
        ${SHARED_SOURCES}
    )

    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # --- 添加预编译头文件 ---
    target_precompile_headers(
        ${TARGET_NAME}
        PRIVATE
        src/pch.hpp
    )

    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        nlohmann_json::nlohmann_json
        SQLite::SQLite3
    )

    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CONFIG_FILE_SOURCE}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/mapping.json"
        COMMENT "Copying mapping.json for ${TARGET_NAME}"
    )
endfunction()

# ===================================================================
# 定义目标
# ===================================================================
message(STATUS "Configuring 'workout_tracker_app'...")
configure_workout_target(workout_tracker_app src/main_app.cpp)

message(STATUS "Configuring 'workout_tracker_cli'...")
configure_workout_target(workout_tracker_cli src/main_cli.cpp)

message(STATUS "Executables will be created at: ${CMAKE_BINARY_DIR}")