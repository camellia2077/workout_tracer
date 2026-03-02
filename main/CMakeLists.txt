# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.15)

# 定义项目名称、版本和语言
project(WorkoutParser VERSION 1.0 LANGUAGES CXX)

# 设置C++标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 查找整个项目都需要的库 ---
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)


# ===================================================================
# 目标 1: 交互式菜单应用 (Interactive App)
# ===================================================================

# --- 仅为交互式应用收集其所需的源文件 ---
message(STATUS "Gathering sources for 'workout_tracker_app'...")
file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB REPROCESSOR_SOURCES 
    "reprocessor/*.cpp"
    "reprocessor/volume_calculator/*.cpp"
    "reprocessor/date_processor/*.cpp"
    "reprocessor/log_formatter/*.cpp"
    "reprocessor/log_parser/*.cpp"
    "reprocessor/name_mapper/*.cpp"
    "reprocessor/validator/*.cpp"
)
file(GLOB DB_INSERTER_SOURCES 
    "db_inserter/*.cpp"
    "db_inserter/_internal/*.cpp"
)
file(GLOB CONTROLLER_SOURCES "controller/*.cpp")

add_executable(
    workout_tracker_app          # 可执行文件名
    main_app.cpp                 # 入口文件
    ${COMMON_SOURCES}
    ${REPROCESSOR_SOURCES}
    ${DB_INSERTER_SOURCES}
    ${CONTROLLER_SOURCES}
)

# ]为交互式应用设置头文件路径 
target_include_directories(
    workout_tracker_app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 为交互式应用链接库
target_link_libraries(
    workout_tracker_app
    PRIVATE 
    nlohmann_json::nlohmann_json 
    SQLite::SQLite3 
)


# ===================================================================
# 目标 2: 命令行工具 (Command-Line Tool)
# ===================================================================

# --- 仅为命令行工具收集其所需的源文件 ---
message(STATUS "Gathering sources for 'workout_tracker_cli'...")
file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB REPROCESSOR_SOURCES 
    "reprocessor/*.cpp"
    "reprocessor/volume_calculator/*.cpp"
    "reprocessor/date_processor/*.cpp"
    "reprocessor/log_formatter/*.cpp"
    "reprocessor/log_parser/*.cpp"
    "reprocessor/name_mapper/*.cpp"
    "reprocessor/validator/*.cpp"
)
file(GLOB DB_INSERTER_SOURCES 
    "db_inserter/*.cpp"
    "db_inserter/_internal/*.cpp"
)
file(GLOB CONTROLLER_SOURCES "controller/*.cpp")


add_executable(
    workout_tracker_cli          # 可执行文件名
    main_cli.cpp                 # 入口文件
    ${COMMON_SOURCES}
    ${REPROCESSOR_SOURCES}
    ${DB_INSERTER_SOURCES}
    ${CONTROLLER_SOURCES}
)

# 为命令行工具设置头文件路径
target_include_directories(
    workout_tracker_cli
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 为命令行工具链接库
target_link_libraries(
    workout_tracker_cli
    PRIVATE 
    nlohmann_json::nlohmann_json 
    SQLite::SQLite3 
)

message(STATUS "Executables will be created at: ${CMAKE_BINARY_DIR}")