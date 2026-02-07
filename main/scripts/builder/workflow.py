# scripts/builder/workflow.py

import os
import sys
import time
from pathlib import Path
from .common import Color, print_header, run_command
from .linter import run_incremental_lint
import tomllib

def execute_build_workflow(args):
    """核心构建流程。"""
    start_time = time.monotonic()

    # 因为脚本在 scripts/builder/ 目录下，所以需要向上三级才能找到项目根目录
    script_file = Path(__file__).resolve()
    project_root = script_file.parent.parent.parent.resolve()
    scripts_dir = project_root / "main" / "scripts"
    
    # Load TOML config
    config_path = scripts_dir / "build_config.toml"
    config = {}
    if config_path.exists():
        with open(config_path, "rb") as f:
            config = tomllib.load(f)
    
    # Merge config with CLI args (CLI takes priority)
    compiler_type = args.compiler or config.get("compiler", {}).get("type", "gcc")
    warning_level = args.warnings or config.get("warnings", {}).get("level", "none")
    
    # Overwrite warning level for full.sh vs fast.sh if not explicitly set
    # User requested: fast=just compile, full=strictest
    if args.fast and not args.warnings:
        warning_level = "none"
    elif not args.fast and not args.warnings and not args.debug:
        # Default for full.sh (which has no special flags normally) is strict
        warning_level = "strict"

    build_type = "Debug" if args.debug or args.lint else "Release"
    
    # 根据参数选择构建目录名
    if args.lint:
        build_dir_name = "build_tidy"
    elif args.debug:
        build_dir_name = "build_debug"
    elif args.fast:
        build_dir_name = "build_fast"
    else:
        build_dir_name = "build"
        
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
    print_header(f"Configuring project with {compiler_type.upper()} and {warning_level.upper()} warnings")
    
    cmake_configure_command = [
        "cmake",
        "..",
        "-G", "Ninja",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        f"-DWARNING_LEVEL={warning_level.upper()}"
    ]
    
    # Set compiler
    if compiler_type == "clang":
        cmake_configure_command.extend([
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++"
        ])
    else:
        cmake_configure_command.extend([
            "-DCMAKE_C_COMPILER=gcc",
            "-DCMAKE_CXX_COMPILER=g++"
        ])

    if args.fast:
        cmake_configure_command.append("-DFAST_BUILD=ON")
    
    run_command(cmake_configure_command, cwd=build_dir)

    # 如果指定了 --format
    if args.format:
        print_header("Running clang-format")
        run_command(["cmake", "--build", ".", "--target", "format"], cwd=build_dir)

    # If --lint is specified
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
