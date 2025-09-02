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
        files_to_copy = { self.config.EXE_NAME: "可执行文件", self.config.CONFIG_NAME: "配置文件" }
        for file_name, desc in files_to_copy.items():
            source = os.path.join(self.config.BUILD_DIR, file_name)
            # [MODIFIED] 复制到新的测试运行目录
            dest = os.path.join(self.test_run_dir, file_name)
            if not os.path.exists(source):
                print(f"  {RED}错误: 源文件未找到 '{source}'. 请检查 config.py。{RESET}")
                return False
            shutil.copy(source, dest)
            print(f"  {GREEN}{desc} '{file_name}' 已成功复制到 '{self.test_run_dir}'。{RESET}")
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
                # ... (日志写入逻辑保持不变)
                return False
        except Exception as e:
            print(f"  ... {RED}CRASHED{RESET}\n      错误: {e}")
            return False