# main_test.py
import os
import sys
from config_manager import ConfigManager
from test_cleanup import Cleaner
from test_runner import TestRunner

# ANSI color codes
GREEN = '\033[92m'
RED = '\033[91m'
CYAN = '\033[96m'
YELLOW = '\033[93m'
RESET = '\033[0m'

def print_usage():
    print(f"\n{YELLOW}Usage: python {os.path.basename(__file__)} [command]{RESET}")
    print("Commands:")
    print(f"  {CYAN}test{RESET}      - 清理工作区并执行所有测试。")
    print(f"  {CYAN}clean{RESET}     - 只执行清理工作区操作。")

def main():
    if os.name == 'nt':
        os.system('color')
    
    args = sys.argv[1:]
    if not args:
        print_usage()
        sys.exit(1)
        
    command = args[0].lower()
    script_dir = os.path.dirname(os.path.abspath(__file__))
    toml_path = os.path.join(script_dir, "test_config.toml")
    
    try:
        config = ConfigManager.load(toml_path)
    except Exception as e:
        print(f"{RED}错误: 加载配置失败: {e}{RESET}")
        sys.exit(1)

    print(f"\n{CYAN}========== Workout Tracker CLI Test Utility =========={RESET}")
    print(f"测试输出将全部保存在: {config.test_run_dir}")
    print(f"执行命令: '{command}'")

    cleaner = Cleaner(config.test_run_dir)

    if command == 'clean':
        if not cleaner.run():
            print(f"\n{RED}❌ 清理步骤失败。{RESET}")
            sys.exit(1)
        print(f"\n{GREEN}✅ 清理操作成功完成!{RESET}")

    elif command == 'test':
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
        print(f"{GREEN}   请检查 '{os.path.join(config.test_run_dir, 'output_file', 'md')}' 目录以验证最终报告。{RESET}")

    else:
        print(f"\n{RED}错误: 未已知命令 '{command}'{RESET}")
        print_usage()
        sys.exit(1)

if __name__ == '__main__':
    main()