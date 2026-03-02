#!/bin/bash

# --- 自动化编译脚本 (仅编译) ---
# 在MSYS2 UCRT64终端中运行此脚本

# set -e: 如果任何命令失败，立即退出脚本
# set -u: 如果使用未定义的变量，视为错误并退出
set -eu

# 获取脚本文件所在的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# 切换到该目录，确保所有后续操作都在正确的项目根目录下执行
cd "$SCRIPT_DIR"
echo "Switched to script directory: $SCRIPT_DIR"

# 定义构建目录的名称
BUILD_DIR="build"

# 1. 清理并创建构建目录
echo -e "\n--- Cleaning and preparing build directory ---"
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    echo "Removed existing build directory."
fi
mkdir "$BUILD_DIR"
echo "Build directory '$BUILD_DIR' created."

# 2. 运行 CMake 来配置项目
echo -e "\n--- Configuring project with CMake ---"
cd "$BUILD_DIR"
cmake ..

# 3. 编译项目
echo -e "\n--- Compiling project ---"
cmake --build .

echo -e "\n--- Build process finished successfully! ---"
echo "Executable is located at: $SCRIPT_DIR/$BUILD_DIR/workout_tracker.exe"