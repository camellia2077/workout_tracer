#!/bin/bash

# --- 启动器脚本 ---
# 切换到脚本所在目录，并执行 Python 编译脚本

# set -e: 如果任何命令失败，立即退出脚本
# set -u: 如果使用未定义的变量，视为错误并退出
set -eu

# 获取脚本文件所在的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# 切换到该目录，确保 Python 脚本在正确的项目根目录下执行
cd "$SCRIPT_DIR"

# 执行 Python 编译脚本
python build.py
