@echo off
cd /d %~dp0

python run.py --suite workout_calculator --build-dir build_fast --no-format-on-success %*

exit /b %ERRORLEVEL%
