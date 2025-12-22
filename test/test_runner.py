# test_runner.py

import os
import shutil
import subprocess
import time
import re

# 定义ANSI颜色代码
GREEN = '\033[92m'
RED = '\033[91m'
CYAN = '\033[96m'
RESET = '\033[0m'

class TestRunner:
    """一个负责在隔离目录中执行所有C++应用测试步骤的类。"""
    def __init__(self, config, test_run_dir):
        """
        初始化TestRunner。
        :param config: 包含所有路径和文件名的配置模块。
        :param test_run_dir: 本次测试运行的根目录，所有文件都将在这里创建。
        """
        self.config = config
        self.test_run_dir = test_run_dir # <<< 这是新的工作根目录
        self.exe_path = os.path.join(self.test_run_dir, self.config.EXE_NAME)

    def run_all(self):
        """按顺序执行所有测试步骤。"""
        os.makedirs(self.test_run_dir, exist_ok=True)
        
        test_steps = [
            {"method": self._prepare_executable, "name": "准备依赖文件"},
            {"method": self._run_validation_test, "name": "验证测试"},
            {"method": self._run_conversion_test, "name": "转换测试"},
            {"method": self._run_insertion_test, "name": "数据库插入测试"},
            {"method": self._run_export_test, "name": "报告导出测试"},
        ]
        
        for step in test_steps:
            if not step["method"]():
                print(f"\n{RED}❌ {step['name']}失败。请检查日志 '{self.test_run_dir}' 获取详情。{RESET}")
                return False
                
        return True

    def _prepare_executable(self):
        print(f"{CYAN}--- 2. Preparing Dependencies ---{RESET}")
        
        # ---------------------------------------------------------
        # 1. 复制可执行文件 (.exe)
        # ---------------------------------------------------------
        source_exe = os.path.join(self.config.BUILD_DIR, self.config.EXE_NAME)
        dest_exe = os.path.join(self.test_run_dir, self.config.EXE_NAME)

        if not os.path.exists(source_exe):
            print(f"  {RED}错误: 可执行文件未找到 '{source_exe}'. 请检查 config.py 及编译输出。{RESET}")
            return False
            
        shutil.copy(source_exe, dest_exe)
        print(f"  {GREEN}可执行文件 '{self.config.EXE_NAME}' 已成功复制到 '{self.test_run_dir}'。{RESET}")

        # ---------------------------------------------------------
        # 2. 复制 config 文件夹 (包含 mapping.json)
        # ---------------------------------------------------------
        # 依据 CMake 的 POST_BUILD 操作，config 文件夹会在 build 目录生成
        source_config_dir = os.path.join(self.config.BUILD_DIR, "config")
        dest_config_dir = os.path.join(self.test_run_dir, "config")

        if not os.path.exists(source_config_dir):
            print(f"  {RED}错误: Config 目录未找到 '{source_config_dir}'。{RESET}")
            print(f"  {RED}      请确保 CMake 构建成功且执行了 POST_BUILD 复制操作。{RESET}")
            return False

        # 如果目标目录已存在，先删除，确保文件是最新的
        if os.path.exists(dest_config_dir):
            shutil.rmtree(dest_config_dir)
        
        try:
            shutil.copytree(source_config_dir, dest_config_dir)
            print(f"  {GREEN}配置目录 'config' (含配置文件) 已成功复制到 '{self.test_run_dir}'。{RESET}")
        except Exception as e:
             print(f"  {RED}错误: 复制 config 目录失败: {e}{RESET}")
             return False

        return True

    def _run_validation_test(self):
        print(f"{CYAN}--- 3. Running Validation Test ---{RESET}")
        return self._execute_command(["validate", self.config.INPUT_DIR], "validation_test.log")

    def _run_conversion_test(self):
        print(f"{CYAN}--- 4. Running Conversion Test ---{RESET}")
        return self._execute_command(["convert", self.config.INPUT_DIR], "conversion_test.log")

    def _run_insertion_test(self):
        print(f"{CYAN}--- 5. Running Database Insertion Test ---{RESET}")
        # [MODIFIED] JSON目录现在也在测试运行目录内
        json_dir = os.path.join(self.test_run_dir, 'reprocessed_json')
        if not os.path.exists(json_dir):
            print(f"  {RED}错误: 未找到 '{json_dir}' 目录。转换步骤可能已失败。{RESET}")
            return False
        return self._execute_command(["insert", json_dir], "insertion_test.log")

    def _run_export_test(self):
        print(f"{CYAN}--- 6. Running Report Export Test ---{RESET}")
        return self._execute_command(["export"], "export_test.log")

    def _execute_command(self, args, log_filename):
        full_command = [self.exe_path] + args
        # [MODIFIED] 日志文件也直接保存在测试运行目录
        log_path = os.path.join(self.test_run_dir, log_filename)
        
        print(f"  -> 执行命令: {' '.join(full_command)}")
        print(f"     工作目录: {self.test_run_dir}")
        print(f"     日志文件: {log_path}", end='', flush=True)

        try:
            # [MODIFIED] C++程序的CWD现在是我们的测试运行目录
            result = subprocess.run(
                full_command, capture_output=True, text=True, encoding='utf-8', 
                errors='ignore', cwd=self.test_run_dir
            )
            if result.returncode == 0:
                print(f"  ... {GREEN}OK{RESET}")
                return True
            else:
                print(f"  ... {RED}FAILED{RESET}")
                # 将错误输出写入日志，方便调试
                with open(log_path, 'a', encoding='utf-8') as f:
                    f.write("\n\n=== STDERR ===\n")
                    f.write(result.stderr)
                return False
        except Exception as e:
            print(f"  ... {RED}CRASHED{RESET}\n      错误: {e}")
            return False