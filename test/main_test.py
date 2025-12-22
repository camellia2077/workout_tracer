# main_test.py

import os
import sys

import config
from test_cleanup import Cleaner
from test_runner import TestRunner

# 定义ANSI颜色代码
GREEN = '\033[92m'
RED = '\033[91m'
CYAN = '\033[96m'
YELLOW = '\033[93m'
RESET = '\033[0m'

def print_usage():
    """打印脚本的用法说明。"""
    print(f"\n{YELLOW}Usage: python {os.path.basename(__file__)} [command]{RESET}")
    print("Commands:")
    print(f"  {CYAN}test{RESET}      - 清理工作区并执行所有测试。")
    print(f"  {CYAN}clean{RESET}     - 只执行清理工作区操作。")

def main():
    """主函数，协调整个测试流程。"""
    if os.name == 'nt':
        os.system('color')
    
    args = sys.argv[1:]
    if not args:
        print_usage()
        sys.exit(1)
    command = args[0].lower()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # [MODIFIED] 决定测试输出的父目录
    if config.TEST_OUTPUT_PARENT_DIR:
        output_parent_dir = os.path.abspath(config.TEST_OUTPUT_PARENT_DIR)
    else:
        output_parent_dir = script_dir
        
    # [MODIFIED] 组装最终的、统一的测试运行目录
    test_run_dir = os.path.join(output_parent_dir, config.TEST_OUTPUT_DIR_NAME)

    print(f"\n{CYAN}========== Workout Tracker CLI Test Utility =========={RESET}")
    print(f"测试输出将全部保存在: {test_run_dir}")
    print(f"执行命令: '{command}'")

    # [MODIFIED] 实例化 Cleaner，目标是删除整个测试运行目录
    cleaner = Cleaner(test_run_dir)

    if command == 'clean':
        if not cleaner.run():
            print(f"\n{RED}❌ 清理步骤失败。{RESET}")
            sys.exit(1)
        print(f"\n{GREEN}✅ 清理操作成功完成!{RESET}")

    elif command == 'test':
        if not cleaner.run():
            print(f"\n{RED}❌ 清理步骤失败，测试终止。{RESET}")
            sys.exit(1)
        
        # [MODIFIED] 实例化 TestRunner，让它在专用的测试目录中工作
        runner = TestRunner(config, test_run_dir)
        if not runner.run_all():
            print(f"\n{RED}❌ 测试流程失败，已终止。{RESET}")
            sys.exit(1)

        print(f"\n{GREEN}✅ 所有测试步骤成功完成!{RESET}")
        print(f"{GREEN}   请检查 '{os.path.join(test_run_dir, 'output_file', 'md')}' 目录以验证最终报告。{RESET}")

    else:
        print(f"\n{RED}错误: 未知命令 '{command}'{RESET}")
        print_usage()
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
    main()