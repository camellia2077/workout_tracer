# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(WorkoutParser VERSION 1.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. 查找依赖 ---
# [MODIFIED] 移除 nlohmann_json，改为查找 cJSON
find_package(cJSON REQUIRED)
find_package(SQLite3 REQUIRED)

# --- 2. 引入自定义 CMake 模块 (核心步骤) ---
# 将 cmake 目录加入模块搜索路径，方便直接 include
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 加载源文件列表
include(SourceFileCollection)

# 加载编译设置函数
include(TargetSetup)

# --- 3. 应用设置 ---
setup_compiler_flags()

# --- 4. 生成目标 ---
message(STATUS "Configuring 'workout_tracker_cli'...")

# 调用我们在 TargetSetup.cmake 中定义的函数
configure_workout_target(
    workout_tracker_cli 
    src/main_cli.cpp 
    "${ALL_SOURCES}"
)