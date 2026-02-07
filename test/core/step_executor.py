# step_executor.py
import os
import subprocess
from core.models import TestConfig

# ANSI color codes
GREEN = '\033[92m'
RED = '\033[91m'
RESET = '\033[0m'

class StepExecutor:
    """负责执行单个测试步骤命令并记录输出。"""
    def __init__(self, config: TestConfig):
        self.config = config
        self._step_counter = 0

    def execute(self, args, log_filename):
        """执行命令，返回是否成功。"""
        full_command = [self.config.exe_path] + args
        log_path = os.path.join(self.config.test_run_dir, log_filename)
        
        print(f"  -> 执行命令: {' '.join(full_command)}")
        print(f"     工作目录: {self.config.test_run_dir}")
        print(f"     日志文件: {log_path}", end='', flush=True)

        EXIT_CODE_MAP = {
            1: "Invalid Arguments",
            2: "Validation Failed",
            3: "File Not Found (or Invalid Path)",
            4: "Database Error",
            5: "Processing Error",
            99: "Unknown Error"
        }

        try:
            result = subprocess.run(
                full_command, capture_output=True, text=True, encoding='utf-8', 
                errors='ignore', cwd=self.config.test_run_dir
            )
            
            # 记录到 pyoutput
            self._record_output(full_command, result, log_filename)

            if result.returncode == 0:
                print(f"  ... {GREEN}OK{RESET}")
                return True
            else:
                error_desc = EXIT_CODE_MAP.get(result.returncode, "Unexpected Error")
                print(f"  ... {RED}FAILED{RESET} ({error_desc}, Code: {result.returncode})")
                
                # 将错误输出写入日志，方便调试
                with open(log_path, 'a', encoding='utf-8') as f:
                    f.write(f"\n\n=== ERROR: {error_desc} (Exit Code: {result.returncode}) ===\n")
                    f.write(result.stderr)
                return False
        except Exception as e:
            print(f"  ... {RED}CRASHED{RESET}\n      错误: {e}")
            return False

    def _record_output(self, command, result, log_filename):
        """记录命令输入和输出到 pyoutput 目录。"""
        self._step_counter += 1
        output_file = os.path.join(
            self.config.py_output_dir, 
            f"step_{self._step_counter}_{log_filename.replace('.log', '.txt')}"
        )
        
        os.makedirs(self.config.py_output_dir, exist_ok=True)
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("=== COMMAND ===\n")
            f.write(" ".join(command) + "\n\n")
            f.write("=== STDOUT ===\n")
            f.write(result.stdout + "\n\n")
            f.write("=== STDERR ===\n")
            f.write(result.stderr + "\n")
