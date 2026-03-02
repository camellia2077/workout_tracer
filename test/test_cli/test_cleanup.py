# test_cleanup.py

import os
import shutil

# 定义ANSI颜色代码
GREEN = '\033[92m'
RED = '\033[91m'
CYAN = '\033[96m'
RESET = '\033[0m'

class Cleaner:
    """一个负责清理工作区的类。"""
    def __init__(self, work_dir, dirs_to_delete, files_to_delete):
        """
        初始化Cleaner。
        :param work_dir: 要进行清理的工作目录。
        :param dirs_to_delete: 需要删除的目录列表。
        :param files_to_delete: 需要删除的文件列表。
        """
        self.work_dir = work_dir
        self.dirs_to_delete = dirs_to_delete
        self.files_to_delete = files_to_delete

    def run(self):
        """执行清理操作。"""
        print(f"{CYAN}--- 1. Cleaning Workspace in '{self.work_dir}' ---{RESET}")
        success = True
        
        for dir_name in self.dirs_to_delete:
            dir_path = os.path.join(self.work_dir, dir_name)
            if os.path.exists(dir_path):
                try:
                    shutil.rmtree(dir_path)
                    print(f"  {GREEN}已删除旧的目录: {dir_path}{RESET}")
                except OSError as e:
                    print(f"  {RED}删除目录 '{dir_path}' 时出错: {e}{RESET}")
                    success = False
                    
        for file_name in self.files_to_delete:
            file_path = os.path.join(self.work_dir, file_name)
            if os.path.exists(file_path):
                try:
                    os.remove(file_path)
                    print(f"  {GREEN}已删除旧的文件: {file_path}{RESET}")
                except OSError as e:
                    print(f"  {RED}删除文件 '{file_path}' 时出错: {e}{RESET}")
                    success = False
        
        if success:
            print(f"  {GREEN}工作区清理完毕。{RESET}")
        else:
            print(f"  {RED}清理过程中发生错误。{RESET}")
            
        return success