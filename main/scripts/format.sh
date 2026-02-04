#!/bin/bash

# --- Clang-Format 启动器 ---
# 切换到脚本所在目录，执行 Python 格式化命令

set -eu

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

echo "--- Running Clang-Format ---"
python build.py --format
