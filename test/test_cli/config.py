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

# [重要] 测试输出目录
# -> 如果设置为一个路径 (例如 r"C:\temp\test_run")，所有输出将保存在那里。
# -> 如果设置为空字符串 ""，所有输出将保存在脚本当前所在的目录。
TEST_OUTPUT_DIR = r"C:\Computer\my_github\github_cpp\workout_calculator\mytest"

# ===================================================================
# 文件名配置
# ===================================================================

# 可执行文件名
EXE_NAME = 'workout_tracker_cli.exe'
# 映射文件名
CONFIG_NAME = 'mapping.json'
# 数据库文件名
DB_NAME = 'workout_logs.sqlite3'

# ===================================================================
# 清理任务配置
# ===================================================================

# 在清理阶段需要删除的目录列表
DIRS_TO_DELETE = [
    'test_output',
    'reprocessed_json',
    'output_file'
]

# 在清理阶段需要删除的文件列表
FILES_TO_DELETE = [
    EXE_NAME, 
    CONFIG_NAME,
    DB_NAME 
]