# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.15) 

# 定义项目名称、版本和语言
project(WorkoutParser VERSION 1.0 LANGUAGES CXX) 

# 设置C++标准为 C++17
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# --- 为Release模式添加优化选项 ---
# -Os: 优化生成文件的大小 (Optimize for size)
# -s: 移除符号信息 (Strip symbols)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")

# --- 查找整个项目都需要的库 ---
find_package(nlohmann_json REQUIRED) 
find_package(SQLite3 REQUIRED) 

# --- 定义配置文件的路径 ---
set(CONFIG_FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/config/mapping.json) 

# ===================================================================
# 现代CMake：显式列出所有源文件，而不是使用 file(GLOB)
# ===================================================================

# 将所有共享的源文件定义在一个变量中，以避免代码重复
set(SHARED_SOURCES
    # Common sources
    src/common/JsonReader.cpp
    src/common/TxtFileReader.cpp
    
    # Reprocessor sources
    src/reprocessor/date_processor/DateProcessor.cpp
    src/reprocessor/log_formatter/LogFormatter.cpp
    src/reprocessor/log_parser/LogParser.cpp
    src/reprocessor/name_mapper/ProjectNameMapper.cpp
    src/reprocessor/validator/Validator.cpp
    src/reprocessor/volume_calculator/VolumeCalculator.cpp
    src/reprocessor/Reprocessor.cpp
    
    # DB Inserter sources
    src/db_inserter/DataManager.cpp
    src/db_inserter/_internal/Database.cpp
    src/db_inserter/_internal/LogRepository.cpp
    
    # Controller sources
    src/controller/ActionHandler.cpp
)

# ===================================================================
# 封装公共配置，避免代码重复
# ===================================================================
function(configure_workout_target TARGET_NAME ENTRY_POINT_FILE)
    # 添加可执行文件，将入口文件和共享源文件列表组合
    add_executable(
        ${TARGET_NAME}
        ${ENTRY_POINT_FILE}
        ${SHARED_SOURCES}
    )

    # 设置头文件路径
    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # 链接库
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE 
        nlohmann_json::nlohmann_json 
        SQLite::SQLite3 
    )

    # 添加构建后命令，将mapping.json复制到可执行文件目录
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
# 目标 1: 交互式菜单应用 (Interactive App)
# ===================================================================
message(STATUS "Configuring 'workout_tracker_app'...")
configure_workout_target(workout_tracker_app src/main_app.cpp)

# ===================================================================
# 目标 2: 命令行工具 (Command-Line Tool)
# ===================================================================
message(STATUS "Configuring 'workout_tracker_cli'...")
configure_workout_target(workout_tracker_cli src/main_cli.cpp)

message(STATUS "Executables will be created at: ${CMAKE_BINARY_DIR}")
