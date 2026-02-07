# run.py
"""
Unified entry point for workout tracker CLI testing.
Accepts build directory as a command-line argument.
"""
import os
import sys
import argparse
from core.config_manager import ConfigManager
from core.test_cleanup import Cleaner
from core.test_runner import TestRunner

# ANSI color codes
GREEN = '\033[92m'
RED = '\033[91m'
CYAN = '\033[96m'
YELLOW = '\033[93m'
RESET = '\033[0m'

def main():
    if os.name == 'nt':
        os.system('color')
    
    parser = argparse.ArgumentParser(description='Workout Tracker CLI Test Utility')
    parser.add_argument('command', choices=['test', 'clean'], 
                       help='Command to execute: test (run all tests) or clean (cleanup only)')
    parser.add_argument('--build-dir', required=True,
                       help='Path to the build directory containing the executable')
    
    args = parser.parse_args()
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    toml_path = os.path.join(script_dir, "test_config.toml")
    
    try:
        config = ConfigManager.load(toml_path, build_dir_override=args.build_dir)
    except Exception as e:
        print(f"{RED}错误: 加载配置失败: {e}{RESET}")
        sys.exit(1)

    print(f"\n{CYAN}========== Workout Tracker CLI Test Utility =========={RESET}")
    print(f"构建目录: {config.build_dir}")
    print(f"测试输出将全部保存在: {config.test_run_dir}")
    print(f"执行命令: '{args.command}'")

    cleaner = Cleaner(config.test_run_dir)

    if args.command == 'clean':
        if not cleaner.run():
            print(f"\n{RED}❌ 清理步骤失败。{RESET}")
            sys.exit(1)
        print(f"\n{GREEN}✅ 清理操作成功完成!{RESET}")

    elif args.command == 'test':
        # 只有在配置开启且命令为 test 时才自动清理
        if config.options.clean_on_start:
            if not cleaner.run():
                print(f"\n{RED}❌ 清理步骤失败，测试终止。{RESET}")
                sys.exit(1)
        
        runner = TestRunner(config)
        if not runner.run_all():
            print(f"\n{RED}❌ 测试流程失败，已终止。{RESET}")
            sys.exit(1)

        print(f"\n{GREEN}✅ 所有测试步骤成功完成!{RESET}")
        print(f"{GREEN}   请检查 '{os.path.join(config.test_run_dir, 'output', 'reports')}' 目录以验证最终报告。{RESET}")

if __name__ == '__main__':
    main()
