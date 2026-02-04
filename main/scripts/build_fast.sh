#!/bin/bash

# --- Fast Build (No Optimizations) 启动器 ---
# 切换到脚本所在目录，执行 Python 禁优化编译命令

set -eu

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

echo "--- Starting Fast Build (Optimizations Disabled) ---"
python build.py --fast
