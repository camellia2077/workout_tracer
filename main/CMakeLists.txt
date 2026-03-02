# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.15)

# 定义项目名称、版本和语言
project(WorkoutParser VERSION 1.0 LANGUAGES CXX)

# 设置C++标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 自动查找您项目的所有源文件 ---
# (这部分您的写法是正确的，我们保持不变)
file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB_RECURSE REPROCESSOR_SOURCES "reprocessor/*.cpp")
file(GLOB_RECURSE DB_INSERTER_SOURCES "db_inserter/*.cpp")


# --- KEY FIX #1: 查找已安装的库 ---
# 查找 nlohmann_json (您的文件中已有)
find_package(nlohmann_json REQUIRED)
# 查找 SQLite3 (这是您文件中缺少的)
find_package(SQLite3 REQUIRED)


# --- 定义可执行文件 ---
# (这部分您的写法是正确的)
add_executable(
    workout_tracker
    main.cpp
    ${COMMON_SOURCES}
    ${REPROCESSOR_SOURCES}
    ${DB_INSERTER_SOURCES}
)


# --- 为目标添加头文件搜索路径 ---
# (您的“宏观路径法”很好，保持不变。find_package会自动处理库的头文件路径)
target_include_directories(
    workout_tracker
    PRIVATE
    # ${CMAKE_CURRENT_SOURCE_DIR} 是项目根目录的绝对路径
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# --- KEY FIX #2: 链接到找到的库目标 ---
target_link_libraries(
    workout_tracker
    PRIVATE 
    nlohmann_json::nlohmann_json 
    SQLite::SQLite3 # <-- 这是最关键的修复：链接到 SQLite3
)


# 打印出可执行文件位置，方便调试 
message(STATUS "Successfully configured workout_tracker.")
message(STATUS "Executable will be created at: ${CMAKE_BINARY_DIR}/workout_tracker.exe")