@echo off
cd /d %~dp0

call run_workout_calculator.bat %*

exit /b %ERRORLEVEL%
