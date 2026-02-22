@echo off
cd /d %~dp0

python ..\..\run.py --suite workout_calculator --no-format-on-success --build-dir build_fast %*

exit /b %ERRORLEVEL%
