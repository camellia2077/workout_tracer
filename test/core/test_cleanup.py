# test_cleanup.py

import os
import shutil

# 定义ANSI颜色代码
GREEN = '\033[92m'
RED = '\033[91m'
CYAN = '\033[96m'
RESET = '\033[0m'

class Cleaner:
    """一个负责清理整个测试输出目录的类。"""
    def __init__(self, directory_to_delete):
        """
        初始化Cleaner。
        :param directory_to_delete: 需要被完整删除的目录的路径。
        """
        self.dir_to_delete = directory_to_delete

    def run(self):
        """执行清理操作。"""
        print(f"{CYAN}--- 1. Cleaning Workspace ---{RESET}")
        if not os.path.exists(self.dir_to_delete):
            print(f"  {GREEN}工作区已是干净的 (目录 '{self.dir_to_delete}' 不存在)。{RESET}")
            return True
        
        try:
            shutil.rmtree(self.dir_to_delete)
            print(f"  {GREEN}已成功删除旧的测试输出目录: {self.dir_to_delete}{RESET}")
            return True
        except OSError as e:
            print(f"  {RED}删除目录 '{self.dir_to_delete}' 时出错: {e}{RESET}")
            return False