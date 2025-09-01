# test_wc.py

import os
import shutil
import subprocess
import time
import re
import sys

# ===================================================================
# 全局配置
# ===================================================================

# *** 重要: 请确保此路径为您项目中 workout_tracker_cli.exe 所在的实际构建目录 ***
BUILD_DIR = r"C:\Computer\my_github\github_cpp\workout_calculator\workout_calculator\workout_calculator\build"

# *** 重要: 请确保此路径为您的测试数据（RECORD）所在的文件夹 ***
INPUT_DIR = r"C:\Computer\my_github\github_cpp\workout_calculator\RECORD"

# 定义可执行文件名
EXE_NAME = 'workout_tracker_cli.exe'
# 定义配置文件名
CONFIG_NAME = 'mapping.json'
# 定义数据库名
DB_NAME = 'workout_logs.sqlite3'

# [MODIFIED] 定义需要清理的项
DIRS_TO_DELETE = [
    'test_output',
    'reprocessed_json',
    'output_file' # 新增
]
FILES_TO_DELETE = [
    EXE_NAME, 
    CONFIG_NAME,
    DB_NAME 
]

# ===================================================================
# 样式和辅助函数
# ===================================================================

# 定义ANSI颜色代码
GREEN = '\033[92m'
YELLOW = '\033[93m'
RED = '\033[91m'
CYAN = '\033[96m'
RESET = '\033[0m'

def remove_ansi_codes(text):
    """一个辅助函数，用于从字符串中移除ANSI颜色转义码。"""
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)

def execute_command(exe_path, args, log_filename, output_dir):
    """一个通用的命令执行函数。"""
    full_command = [exe_path] + args
    log_path = os.path.join(output_dir, log_filename)
    
    print(f"  -> 执行命令: {' '.join(full_command)}")
    print(f"     日志文件: {log_path}", end='', flush=True)

    try:
        start_time = time.time()
        result = subprocess.run(
            full_command,
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='ignore'
        )
        end_time = time.time()
        duration = end_time - start_time

        clean_stdout = remove_ansi_codes(result.stdout)
        clean_stderr = remove_ansi_codes(result.stderr)

        with open(log_path, 'w', encoding='utf-8') as f:
            f.write(f"--- Command ---\n{' '.join(full_command)}\n\n")
            f.write(f"--- Return Code ---\n{result.returncode}\n\n")
            f.write(f"--- Execution Time ---\n{duration:.2f} seconds\n\n")
            f.write(f"--- STDOUT ---\n{clean_stdout}\n")
            if result.stderr:
                f.write(f"\n--- STDERR ---\n{clean_stderr}\n")

        if result.returncode == 0:
            print(f"  ... {GREEN}OK{RESET} ({duration:.2f}s)")
            return True
        else:
            print(f"  ... {RED}FAILED{RESET} (返回码: {result.returncode})")
            print(f"      请查看日志获取详情: {log_path}")
            return False

    except FileNotFoundError:
        print(f"  ... {RED}CRASHED{RESET}\n      错误: 找不到命令 '{exe_path}'。")
        return False
    except Exception as e:
        print(f"  ... {RED}CRASHED{RESET}\n      错误: {e}")
        with open(log_path, 'w', encoding='utf-8') as f:
            f.write(f"--- Command ---\n{' '.join(full_command)}\n\n")
            f.write(f"--- CRITICAL ERROR ---\n脚本执行命令时发生严重错误。\n错误详情: {e}\n")
        return False

# ===================================================================
# 测试任务
# ===================================================================

def cleanup(base_dir):
    """清理上一次运行留下的文件和文件夹。"""
    print(f"{CYAN}--- 1. Cleaning Workspace ---{RESET}")
    success = True
    for file_name in FILES_TO_DELETE:
        file_path = os.path.join(base_dir, file_name)
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
                print(f"  {GREEN}已删除旧的文件: {file_name}{RESET}")
            except OSError as e:
                print(f"  {RED}删除文件 '{file_path}' 时出错: {e}{RESET}")
                success = False
    for dir_name in DIRS_TO_DELETE:
        dir_path = os.path.join(base_dir, dir_name)
        if os.path.exists(dir_path):
            try:
                shutil.rmtree(dir_path)
                print(f"  {GREEN}已删除旧的目录: {dir_name}{RESET}")
            except OSError as e:
                print(f"  {RED}删除目录 '{dir_path}' 时出错: {e}{RESET}")
                success = False
    return success

def prepare_executable(base_dir):
    """从build目录复制可执行文件和配置文件。"""
    print(f"{CYAN}--- 2. Preparing Dependencies ---{RESET}")
    files_to_copy = { EXE_NAME: "可执行文件", CONFIG_NAME: "配置文件" }
    all_success = True
    for file_name, file_desc in files_to_copy.items():
        source_path = os.path.join(BUILD_DIR, file_name)
        dest_path = os.path.join(base_dir, file_name)
        if not os.path.exists(source_path):
            print(f"  {RED}错误: 源文件未找到 '{source_path}'.{RESET}")
            print(f"  {YELLOW}请检查 'BUILD_DIR' 变量是否设置正确，并确保项目已成功编译。{RESET}")
            all_success = False
            continue
        try:
            shutil.copy(source_path, dest_path)
            print(f"  {GREEN}{file_desc} '{file_name}' 已成功复制。{RESET}")
        except IOError as e:
            print(f"  {RED}复制文件 '{file_name}' 时出错: {e}{RESET}")
            all_success = False
    return all_success

def run_validation_test(base_dir):
    """执行'validate'命令的测试。"""
    print(f"{CYAN}--- 3. Running Validation Test ---{RESET}")
    exe_path = os.path.join(base_dir, EXE_NAME)
    output_dir = os.path.join(base_dir, 'test_output')
    os.makedirs(output_dir, exist_ok=True)
    
    command_args = ["validate", INPUT_DIR]
    return execute_command(exe_path, command_args, "validation_test.log", output_dir)

def run_conversion_test(base_dir):
    """执行'convert'命令的测试。"""
    print(f"{CYAN}--- 4. Running Conversion Test ---{RESET}")
    exe_path = os.path.join(base_dir, EXE_NAME)
    output_dir = os.path.join(base_dir, 'test_output')
    os.makedirs(output_dir, exist_ok=True)
    
    command_args = ["convert", INPUT_DIR]
    return execute_command(exe_path, command_args, "conversion_test.log", output_dir)

def run_insertion_test(base_dir):
    """执行'insert'命令的测试。"""
    print(f"{CYAN}--- 5. Running Database Insertion Test ---{RESET}")
    exe_path = os.path.join(base_dir, EXE_NAME)
    output_dir = os.path.join(base_dir, 'test_output')
    json_input_dir = os.path.join(base_dir, 'reprocessed_json')
    os.makedirs(output_dir, exist_ok=True)
    
    if not os.path.exists(json_input_dir):
        print(f"  {RED}错误: 未找到 'reprocessed_json' 目录。转换步骤可能已失败。{RESET}")
        return False
        
    command_args = ["insert", json_input_dir]
    return execute_command(exe_path, command_args, "insertion_test.log", output_dir)

def run_export_test(base_dir):
    """[NEW] 执行'export'命令的测试。"""
    print(f"{CYAN}--- 6. Running Report Export Test ---{RESET}")
    exe_path = os.path.join(base_dir, EXE_NAME)
    output_dir = os.path.join(base_dir, 'test_output')
    os.makedirs(output_dir, exist_ok=True)
    
    command_args = ["export"]
    return execute_command(exe_path, command_args, "export_test.log", output_dir)

# ===================================================================
# 主函数
# ===================================================================

def main():
    """主函数，协调整个测试流程。"""
    if os.name == 'nt':
        os.system('color')
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    print(f"\n{CYAN}========== 开始测试: Workout Tracker CLI =========={RESET}")

    if not cleanup(script_dir):
        print(f"\n{RED}❌ 清理步骤失败，测试终止。{RESET}")
        sys.exit(1)
        
    if not prepare_executable(script_dir):
        print(f"\n{RED}❌ 准备依赖文件失败，测试终止。{RESET}")
        sys.exit(1)
        
    if not run_validation_test(script_dir):
        print(f"\n{RED}❌ 验证测试失败。请检查 'test_output/validation_test.log' 获取详情。{RESET}")
        sys.exit(1)

    if not run_conversion_test(script_dir):
        print(f"\n{RED}❌ 转换测试失败。请检查 'test_output/conversion_test.log' 获取详情。{RESET}")
        sys.exit(1)
    
    if not run_insertion_test(script_dir):
        print(f"\n{RED}❌ 数据库插入测试失败。请检查 'test_output/insertion_test.log' 获取详情。{RESET}")
        sys.exit(1)
        
    if not run_export_test(script_dir):
        print(f"\n{RED}❌ 报告导出测试失败。请检查 'test_output/export_test.log' 获取详情。{RESET}")
        sys.exit(1)

    print(f"\n{GREEN}✅ 所有测试步骤成功完成!{RESET}")
    print(f"{GREEN}   请检查 'output_file/md' 目录以验证最终报告。{RESET}")

if __name__ == '__main__':
    main()