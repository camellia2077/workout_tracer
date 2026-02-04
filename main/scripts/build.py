# scripts/build.py

import os
import subprocess
import sys
import time
import argparse
import shutil
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

def run_command(command, cwd, stdout=None, stderr=None):
    """在一个指定的目录中执行命令，并检查错误。"""
    cmd_str = ' '.join(map(str, command)) if isinstance(command, list) else command
    print(f"--- Running command: {cmd_str} in '{cwd}'")
    try:
        # 如果提供了 stdout/stderr，则重定向
        if stdout or stderr:
            subprocess.run(command, cwd=cwd, check=True, stdout=stdout, stderr=stderr)
        else:
            subprocess.run(command, cwd=cwd, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"{Color.FAIL}!!! Error executing command: {cmd_str}{Color.ENDC}", file=sys.stderr)
        print(f"{Color.FAIL}!!! Error: {e}{Color.ENDC}", file=sys.stderr)
        sys.exit(1)

def run_incremental_lint(build_dir, project_root):
    """逐个处理文件并即时输出结果，防止用户以为卡死"""
    print_header("Running Incremental Clang-Tidy")
    
    src_dir = project_root / "src"
    cpp_files = sorted(list(src_dir.rglob("*.cpp")))
    
    if not cpp_files:
        print(f"{Color.FAIL}No source files found in {src_dir}{Color.ENDC}")
        return

    log_file = build_dir / "clang-tidy.log"
    print(f"Logging full output to: {log_file}")
    
    clang_tidy = shutil.which("clang-tidy")
    if not clang_tidy:
        print(f"{Color.FAIL}Error: clang-tidy not found in PATH.{Color.ENDC}")
        return

    issues_found = False
    with open(log_file, "w", encoding='utf-8') as f:
        for i, cpp_file in enumerate(cpp_files):
            rel_path = cpp_file.relative_to(project_root)
            display_path = str(rel_path).replace('\\', '/')
            
            # 状态提示仅留在控制台，不写入日志
            print(f"[{i+1}/{len(cpp_files)}] Linting {display_path}...{' ' * 20}", end='\r', flush=True)
            
            # 增加 -extra-arg=-fdiagnostics-color=always 以尝试保留颜色（如果用户终端支持）
            cmd = [clang_tidy, "-p", str(build_dir), str(cpp_file), "--quiet"]
            
            result = subprocess.run(cmd, cwd=project_root, capture_output=True, text=True, encoding='utf-8')
            
            if result.stdout:
                issues_found = True
                sys.stdout.write(result.stdout)
                f.write(result.stdout)
            
            if result.stderr:
                # 编译器警告有时在 stderr
                sys.stderr.write(result.stderr)
                f.write(result.stderr)

    print(f"\n{Color.OKGREEN if not issues_found else Color.HEADER}Clang-tidy completed.{Color.ENDC}")

def main():
    """配置并构建项目的主函数。"""
    parser = argparse.ArgumentParser(description="Workout Calculator Build Script")
    parser.add_argument("--debug", action="store_true", help="Build in Debug mode")
    parser.add_argument("--lint", action="store_true", help="Run clang-tidy and save output to log")
    parser.add_argument("--format", action="store_true", help="Run clang-format on all source files")
    parser.add_argument("--fast", action="store_true", help="Build with optimizations disabled")
    args = parser.parse_args()

    start_time = time.monotonic()

    # 因为脚本在 scripts/ 目录下，所以需要向上两级才能找到项目根目录
    project_root = Path(__file__).parent.parent.resolve()
    
    build_type = "Debug" if args.debug or args.lint else "Release"
    # [MODIFIED] 如果是 --fast，我们也放在 build 目录，或者考虑 build_fast? 
    # 为了保持简单，--fast 覆盖 build 目录即可，或者让它区分开
    build_dir_name = "build_debug" if build_type == "Debug" else ("build_fast" if args.fast else "build")
    build_dir = project_root / build_dir_name
    
    # 在Windows上启用ANSI颜色代码
    if sys.platform.startswith('win'):
        os.system('color')
        
    os.chdir(project_root)
    print(f"Switched to project root directory: {project_root}")

    # 准备构建目录
    print_header(f"Preparing {build_type} build directory {'(Fast Mode)' if args.fast else ''}")
    if not build_dir.exists():
        print(f"Build directory '{build_dir.name}' not found. Creating it...")
        build_dir.mkdir()
    else:
        print(f"Using existing build directory '{build_dir.name}'.")

    # 配置项目
    print_header(f"Configuring project with CMake+Ninja for {build_type} build")
    cmake_configure_command = [
        "cmake",
        "..",
        "-G", "Ninja",
        f"-DCMAKE_BUILD_TYPE={build_type}"
    ]
    if args.fast:
        cmake_configure_command.append("-DFAST_BUILD=ON")
    
    run_command(cmake_configure_command, cwd=build_dir)

    # 如果指定了 --format
    if args.format:
        print_header("Running clang-format")
        run_command(["cmake", "--build", ".", "--target", "format"], cwd=build_dir)

    # 如果指定了 --lint
    if args.lint:
        run_incremental_lint(build_dir, project_root)

    # 编译项目 (如果不是只为了 format/lint)
    if not (args.format or args.lint):
        print_header(f"Compiling project in {build_type} mode with Ninja")
        cmake_build_command = [
            "cmake",
            "--build", ".",
            "--config", build_type
        ]
        run_command(cmake_build_command, cwd=build_dir)

    end_time = time.monotonic()
    duration = end_time - start_time
    
    print_header("Process finished!")
    if not (args.format or args.lint):
        print(f"{Color.OKGREEN}Executables are located at: {build_dir}{Color.ENDC}")
    print(f"{Color.OKGREEN}Total time: {duration:.2f} seconds{Color.ENDC}")

if __name__ == "__main__":
    main()