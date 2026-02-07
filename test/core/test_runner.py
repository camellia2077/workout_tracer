# test_runner.py
import os
import shutil
from core.models import TestConfig
from core.step_executor import StepExecutor

# ANSI color codes
CYAN = '\033[96m'
GREEN = '\033[92m'
RED = '\033[91m'
RESET = '\033[0m'

class TestRunner:
    """负责协调整个测试流程的类。"""
    def __init__(self, config: TestConfig):
        self.config = config
        self.executor = StepExecutor(config)

    def run_all(self):
        """按顺序执行所有测试步骤。"""
        os.makedirs(self.config.test_run_dir, exist_ok=True)
        
        test_steps = [
            {"method": self._prepare_dependencies, "name": "准备依赖文件"},
            {"method": self._run_validation_test, "name": "验证测试"},
            {"method": self._run_conversion_test, "name": "转换测试"},
            {"method": self._run_insertion_test, "name": "数据库插入测试"},
            {"method": self._run_export_test, "name": "报告导出测试"},
        ]
        
        for step in test_steps:
            if not step["method"]():
                print(f"\n{RED}❌ {step['name']}失败。请检查日志 '{self.config.test_run_dir}' 获取详情。{RESET}")
                return False
                
        return True

    def _prepare_dependencies(self):
        print(f"{CYAN}--- 2. Preparing Dependencies ---{RESET}")
        
        for item in self.config.copy_items:
            source_path = os.path.join(self.config.paths.build_dir, item.source_rel)
            dest_path = os.path.join(self.config.test_run_dir, item.dest_rel)

            if not os.path.exists(source_path):
                print(f"  {RED}错误: 源文件/目录未找到 '{source_path}'。{RESET}")
                print(f"  {RED}绝对路径: {os.path.abspath(source_path)}{RESET}")
                return False

            try:
                if item.type == 'file':
                    # Ensure parent dir exists
                    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                    shutil.copy(source_path, dest_path)
                    print(f"  {GREEN}文件 '{item.source_rel}' 已成功复制。{RESET}")
                elif item.type == 'dir':
                    if os.path.exists(dest_path):
                        shutil.rmtree(dest_path)
                    shutil.copytree(source_path, dest_path)
                    print(f"  {GREEN}目录 '{item.source_rel}' 已成功复制。{RESET}")
            except Exception as e:
                print(f"  {RED}错误: 复制 '{item.source_rel}' 失败: {e}{RESET}")
                return False

        return True

    def _run_validation_test(self):
        print(f"{CYAN}--- 3. Running Validation Test ---{RESET}")
        return self.executor.execute(["validate", self.config.paths.input_dir], "validation_test.log")

    def _run_conversion_test(self):
        print(f"{CYAN}--- 4. Running Conversion Test ---{RESET}")
        return self.executor.execute(["convert", self.config.paths.input_dir], "conversion_test.log")

    def _run_insertion_test(self):
        print(f"{CYAN}--- 5. Running Database Insertion Test ---{RESET}")
        json_dir = os.path.join(self.config.test_run_dir, 'output', 'data')
        if not os.path.exists(json_dir):
            print(f"  {RED}错误: 未找到 '{json_dir}' 目录。转换步骤可能已失败。{RESET}")
            return False
        return self.executor.execute(["insert", json_dir], "insertion_test.log")

    def _run_export_test(self):
        print(f"{CYAN}--- 6. Running Report Export Test ---{RESET}")
        return self.executor.execute(["export"], "export_test.log")