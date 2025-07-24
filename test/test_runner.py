import os
import shutil
import subprocess
import sys

# --- 配置区 ---
EXECUTABLE_SOURCE_PATH = r"C:\Computer\my_github\github_cpp\workout_calculator\main\build\workout_tracker_cli.exe"
LOG_SOURCE_DIRECTORY = r"C:\Computer\my_github\github_cpp\workout_calculator\my_test\RECORD"

EXECUTABLE_NAME = "workout_tracker_cli.exe"
DB_NAME = "workouts.sqlite3"
REPROCESSED_DIR = "reprocessed"
MAPPING_JSON_NAME = "mapping.json"


# --- 辅助函数 ---
def print_header(title):
    print("\n" + "=" * 60)
    print(f"  {title}")
    print("=" * 60)

def print_status(message, status):
    print(f"  -> {message:<45} [{status}]")

def full_cleanup():
    """
    完全清理：用于测试开始前，删除所有旧的测试产物，包括可执行文件。
    """
    print("  -> Performing full cleanup (including executable)...")
    if os.path.exists(EXECUTABLE_NAME):
        os.remove(EXECUTABLE_NAME)
    if os.path.exists(DB_NAME):
        os.remove(DB_NAME)
    if os.path.exists(REPROCESSED_DIR):
        shutil.rmtree(REPROCESSED_DIR)

def final_cleanup():
    """
    最终清理：用于测试结束后，保留可执行文件，只删除临时产物。
    """
    print("  -> Performing final cleanup (keeping executable)...")
    if os.path.exists(DB_NAME):
        os.remove(DB_NAME)
    if os.path.exists(REPROCESSED_DIR):
        shutil.rmtree(REPROCESSED_DIR)

def setup():
    """准备测试环境"""
    # 已修改：调用完全清理
    full_cleanup()
    print("  -> Setting up test environment...")
    
    if not os.path.exists(EXECUTABLE_SOURCE_PATH):
        print_status(f"Executable not found at '{EXECUTABLE_SOURCE_PATH}'", "FATAL")
        print("  !!! Please build the C++ project first or update the EXECUTABLE_SOURCE_PATH variable.")
        return False
    shutil.copy(EXECUTABLE_SOURCE_PATH, EXECUTABLE_NAME)
    
    if not os.path.isdir(LOG_SOURCE_DIRECTORY):
        print_status(f"Log source directory not found at '{LOG_SOURCE_DIRECTORY}'", "FATAL")
        return False
        
    print("  -> Setup complete.")
    return True
    
def run_test_case(command, description):
    """运行单个测试命令并返回结果"""
    print(f"\n  Running test: {description}")
    command_str = [arg.replace('\\', '/') for arg in command]
    print(f"  Command: .\\{EXECUTABLE_NAME} {' '.join(command_str)}")
    
    result = subprocess.run([EXECUTABLE_NAME] + command, capture_output=True, text=True, encoding='utf-8')
    
    if result.stdout:
        print(f"  STDOUT:\n---\n{result.stdout.strip()}\n---")
    if result.stderr:
        print(f"  STDERR:\n---\n{result.stderr.strip()}\n---")
    return result

# --- 核心测试逻辑 ---

def run_reprocess_on_all_logs():
    """
    扫描指定目录下的所有.txt文件，并对它们执行-r模式。
    """
    print_header(f"Core Test: Reprocessing all .txt files in '{LOG_SOURCE_DIRECTORY}'")
    if not setup(): return False
    
    try:
        log_files_to_test = [f for f in os.listdir(LOG_SOURCE_DIRECTORY) if f.endswith('.txt')]
    except FileNotFoundError:
        print(f"  Error: The directory '{LOG_SOURCE_DIRECTORY}' was not found during test execution.")
        return False

    if not log_files_to_test:
        print(f"  -> No .txt files found in '{LOG_SOURCE_DIRECTORY}'. Test inconclusive.")
        return True

    all_tests_passed = True
    for log_file in log_files_to_test:
        log_path = os.path.join(LOG_SOURCE_DIRECTORY, log_file)
        
        # 每次测试前清理数据库和输出文件夹，确保结果独立
        if os.path.exists(DB_NAME): os.remove(DB_NAME)
        if os.path.exists(REPROCESSED_DIR): shutil.rmtree(REPROCESSED_DIR)

        cmd = ["-r", log_path]
        result = run_test_case(cmd, f"Processing file: {log_file}")

        passed = (result.returncode == 0)
        
        print_status(f"Verification for '{log_file}'", "PASS" if passed else "FAIL")
        if not passed:
            all_tests_passed = False
            
    return all_tests_passed

# --- 主函数 ---
if __name__ == "__main__":
    if not os.path.exists(MAPPING_JSON_NAME):
        print(f"Error: Required configuration file '{MAPPING_JSON_NAME}' not found in the current directory.")
        print("Please create it or copy it here before running the test suite.")
        sys.exit(1)

    print_header("Starting Focused Test Runner")
    
    success = run_reprocess_on_all_logs()

    print_header("Test Suite Summary")
    if success:
        print("  Congratulations! All log files were processed successfully.")
    else:
        print("  One or more log files failed to process. Please review the logs above.")
    
    print("\n" + "=" * 60)
    
    # 已修改：调用最终清理
    final_cleanup()