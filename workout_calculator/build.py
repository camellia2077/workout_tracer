import os
import subprocess
import sys
import time  # 1. 导入 time 模块
from pathlib import Path

def run_command(command, cwd):
    """Executes a command in a specified directory and checks for errors."""
    print(f"--- Running command: {' '.join(command)} in '{cwd}'")
    try:
        is_windows = sys.platform.startswith('win')
        subprocess.run(command, cwd=cwd, check=True, shell=is_windows)
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"!!! Error executing command: {' '.join(command)}", file=sys.stderr)
        print(f"!!! Error: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    """Main function to configure and build the project."""
    # 2. 在 main 函数开始时记录开始时间
    start_time = time.monotonic()

    project_root = Path(__file__).parent.resolve()
    os.chdir(project_root)
    print(f"Switched to script directory: {project_root}")

    build_dir = project_root / "build"

    # 准备构建目录
    print(f"\n--- Preparing build directory ---")
    if not build_dir.exists():
        print(f"Build directory '{build_dir.name}' not found. Creating it...")
        build_dir.mkdir()
    else:
        print(f"Using existing build directory '{build_dir.name}'.")

    # 配置项目
    print(f"\n--- Configuring project with CMake+Ninja for Release build ---")
    cmake_configure_command = [
        "cmake",
        "..",
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release"
    ]
    run_command(cmake_configure_command, cwd=build_dir)

    # 编译项目
    print(f"\n--- Compiling project in Release mode with Ninja ---")
    cmake_build_command = [
        "cmake",
        "--build", ".",
        "--config", "Release"
    ]
    run_command(cmake_build_command, cwd=build_dir)

    print(f"\n--- Build process finished successfully! ---")
    print(f"Executables are located at: {build_dir}")

    # 3. 计算并打印总耗时
    end_time = time.monotonic()
    duration = end_time - start_time
    print(f"\n--- Total build time: {duration:.2f} seconds ---")

if __name__ == "__main__":
    main()
