# scripts/build.py

import os
import subprocess
import sys
import time
from pathlib import Path

# ===================================================================
# 1. 控制台颜色定义
# ===================================================================
class Color:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def print_header(message):
    """打印带有彩色标题格式的日志消息"""
    print(f"\n{Color.HEADER}{Color.BOLD}--- {message} ---{Color.ENDC}")

# ===================================================================
# 核心函数
# ===================================================================

def run_command(command, cwd):
    """在一个指定的目录中执行命令，并检查错误。"""
    print(f"--- Running command: {' '.join(command)} in '{cwd}'")
    try:
        # 直接执行命令，让子进程的彩色输出流到当前终端
        subprocess.run(command, cwd=cwd, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"{Color.FAIL}!!! Error executing command: {' '.join(command)}{Color.ENDC}", file=sys.stderr)
        print(f"{Color.FAIL}!!! Error: {e}{Color.ENDC}", file=sys.stderr)
        sys.exit(1)

def main():
    """配置并构建项目的主函数。"""
    start_time = time.monotonic()

    # [MODIFIED] 因为脚本在 scripts/ 目录下，所以需要向上两级才能找到项目根目录
    project_root = Path(__file__).parent.parent.resolve()
    build_dir = project_root / "build"
    
    # 在Windows上启用ANSI颜色代码
    if sys.platform.startswith('win'):
        os.system('color')
        
    os.chdir(project_root)
    # [MODIFIED] 更新提示信息
    print(f"Switched to project root directory: {project_root}")

    # 准备构建目录
    print_header("Preparing build directory")
    if not build_dir.exists():
        print(f"Build directory '{build_dir.name}' not found. Creating it...")
        build_dir.mkdir()
    else:
        print(f"Using existing build directory '{build_dir.name}'.")

    # 配置项目 - 确保使用Ninja
    # 注意：这里的 ".." 指的是相对于 build 目录的上一级，也就是 project_root，
    # 只要 project_root 计算正确，CMakeLists.txt 就能被找到。
    print_header("Configuring project with CMake+Ninja for Release build")
    cmake_configure_command = [
        "cmake",
        "..",
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release"
    ]
    run_command(cmake_configure_command, cwd=build_dir)

    # 编译项目
    print_header("Compiling project in Release mode with Ninja")
    # cmake --build . 会自动使用上面配置的Ninja
    cmake_build_command = [
        "cmake",
        "--build", ".",
        "--config", "Release"
    ]
    run_command(cmake_build_command, cwd=build_dir)

    end_time = time.monotonic()
    duration = end_time - start_time
    
    print_header("Build process finished successfully!")
    print(f"{Color.OKGREEN}Executables are located at: {build_dir}{Color.ENDC}")
    print(f"{Color.OKGREEN}Total build time: {duration:.2f} seconds{Color.ENDC}")

if __name__ == "__main__":
    main()