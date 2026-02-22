@echo off
cd /d %~dp0

python ..\..\run.py --suite workout_calculator --agent --concise --no-format-on-success %*

exit /b %ERRORLEVEL%
