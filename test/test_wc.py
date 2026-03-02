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
# --- 新增: 定义配置文件名 ---
CONFIG_NAME = 'mapping.json'

# 定义需要清理的文件夹
DIRS_TO_DELETE = [
    'test_output',
    'reprocessed'
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

# ===================================================================
# 测试任务类
# ===================================================================

def cleanup(base_dir):
    """清理上一次运行留下的文件和文件夹。"""
    print(f"{CYAN}--- 1. Cleaning Workspace ---{RESET}")
    success = True
    
    # --- 已修改: 将要删除的文件放入一个列表 ---
    files_to_delete = [EXE_NAME, CONFIG_NAME]

    for file_name in files_to_delete:
        file_path = os.path.join(base_dir, file_name)
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
                print(f"  {GREEN}已删除旧的文件: {file_name}{RESET}")
            except OSError as e:
                print(f"  {RED}删除文件 '{file_path}' 时出错: {e}{RESET}")
                success = False

    # 清理旧的目录
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
    """从build目录复制可执行文件和配置文件到当前目录。"""
    print(f"{CYAN}--- 2. Preparing Dependencies ---{RESET}")
    
    # --- 已修改: 将要复制的文件放入一个字典 ---
    files_to_copy = {
        EXE_NAME: "可执行文件",
        CONFIG_NAME: "配置文件"
    }

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


def run_main_command(base_dir):
    """执行核心的命令行程序并记录日志。"""
    print(f"{CYAN}--- 3. Running Main Command ---{RESET}")
    
    exe_path = os.path.join(base_dir, EXE_NAME)
    output_dir = os.path.join(base_dir, 'test_output')
    
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    
    command_args = ['-o', INPUT_DIR]
    full_command = [exe_path] + command_args
    
    # 生成一个安全的日志文件名
    sanitized_command_name = "workout_tracker_main_test"
    log_path = os.path.join(output_dir, f"{sanitized_command_name}.log")
    
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

        # check=True 的行为通过手动检查返回码来实现
        if result.returncode == 0:
            print(f"  ... {GREEN}OK{RESET} ({duration:.2f}s)")
            return True
        else:
            print(f"  ... {RED}FAILED{RESET} (返回码: {result.returncode})")
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


# 主函数

def main():
    """主函数，协调整个测试流程。"""
    # 在Windows上启用ANSI颜色支持
    if os.name == 'nt':
        os.system('color')
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    print(f"\n{CYAN}========== 开始测试: Workout Tracker CLI =========={RESET}")

    # 依次执行各个步骤
    if not cleanup(script_dir):
        print(f"\n{RED}❌ 清理步骤失败，测试终止。{RESET}")
        sys.exit(1)
        
    if not prepare_executable(script_dir):
        print(f"\n{RED}❌ 准备依赖文件失败，测试终止。{RESET}")
        sys.exit(1)
        
    if not run_main_command(script_dir):
        print(f"\n{RED}❌ 命令执行失败。请检查 'test_output' 目录中的日志文件获取详细信息。{RESET}")
        sys.exit(1)
    
    # 所有步骤成功
    print(f"\n{GREEN}✅ 所有测试步骤完成!{RESET}")
    print(f"{GREEN}   请检查 'test_output' 目录以获取详细的运行日志。{RESET}")

if __name__ == '__main__':
    main()