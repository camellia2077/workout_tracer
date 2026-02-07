# scripts/builder/common.py

import subprocess
import sys

class Color:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def print_header(message):
    """打印带有彩色标题格式的日志消息"""
    print(f"\n{Color.HEADER}{Color.BOLD}--- {message} ---{Color.ENDC}")

def run_command(command, cwd, stdout=None, stderr=None):
    """在一个指定的目录中执行命令，并检查错误。"""
    cmd_str = ' '.join(map(str, command)) if isinstance(command, list) else command
    print(f"--- Running command: {cmd_str} in '{cwd}'")
    try:
        if stdout or stderr:
            subprocess.run(command, cwd=cwd, check=True, stdout=stdout, stderr=stderr)
        else:
            subprocess.run(command, cwd=cwd, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"{Color.FAIL}!!! Error executing command: {cmd_str}{Color.ENDC}", file=sys.stderr)
        print(f"{Color.FAIL}!!! Error: {e}{Color.ENDC}", file=sys.stderr)
        sys.exit(1)
