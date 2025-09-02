# config.py

"""
这是一个中央配置文件，用于存储测试脚本的所有可配置参数。
"""

# ===================================================================
# 核心路径配置
# ===================================================================

# [重要] C++ 项目编译后 workout_tracker_cli.exe 所在的构建目录
BUILD_DIR = r"C:\Computer\my_github\github_cpp\workout_calculator\workout_calculator\workout_calculator\build"

# [重要] 存放原始训练日志 (.txt 文件) 的目录
INPUT_DIR = r"C:\Computer\my_github\github_cpp\workout_calculator\RECORD"

# [重要] 测试输出的根目录
# -> 如果设置为一个路径 (例如 r"C:\temp\my_tests")，所有测试输出将保存在该路径下。
# -> 如果设置为空字符串 ""，所有输出将保存在脚本当前所在的目录。
TEST_OUTPUT_PARENT_DIR = r"C:\Computer\my_github\github_cpp\workout_calculator\mytest"

# [MODIFIED] 定义所有测试产物的总目录名。清理时会直接删除这个目录。
TEST_OUTPUT_DIR_NAME = 'test_output'

# ===================================================================
# 文件名配置 (保持不变)
# ===================================================================
EXE_NAME = 'workout_tracker_cli.exe'
CONFIG_NAME = 'mapping.json'
DB_NAME = 'workout_logs.sqlite3'