# scripts/build.py

import argparse
from builder.workflow import execute_build_workflow

def main():
    """解析命令行参数并启动构建工作流。"""
    parser = argparse.ArgumentParser(description="Workout Calculator Build Script")
    parser.add_argument("--debug", action="store_true", help="Build in Debug mode")
    parser.add_argument("--lint", action="store_true", help="Run clang-tidy and save output to log")
    parser.add_argument("--format", action="store_true", help="Run clang-format on all source files")
    parser.add_argument("--fast", action="store_true", help="Build with optimizations disabled")
    parser.add_argument("--compiler", choices=["gcc", "clang"], help="Compiler to use (overrides TOML)")
    parser.add_argument("--warnings", choices=["none", "strict"], help="Warning level (overrides TOML)")
    args = parser.parse_args()

    # 执行核心逻辑
    execute_build_workflow(args)

if __name__ == "__main__":
    main()