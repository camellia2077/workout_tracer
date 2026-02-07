#!/bin/bash

# --- Clang-Tidy (Check Only) 启动器 ---
# 切换到脚本所在目录，执行 Python 静态检查命令并记录日志

set -eu

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

echo "--- Running Clang-Tidy (Check Only, Log in build_tidy/clang-tidy.log) ---"
python ../../build.py --lint
