@echo off
REM ----------------------------------------------------------------
REM  自动化测试启动脚本 (Windows平台)
REM ----------------------------------------------------------------

REM 设置标题，方便识别
TITLE Workout Tracker - Test Runner

REM 切换到批处理文件所在的目录
REM %~dp0 会扩展为当前批处理文件所在的驱动器和路径
cd /d "%~dp0"

REM 检查Python是否存在
where python >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未在系统中找到 Python。请确保 Python 已安装并已添加到系统 PATH 环境变量中。
    goto :end
)

REM 启动Python测试脚本
python test_wc.py
