@echo off
cd /d %~dp0

rem Usage:
rem   run_workout_calculator.bat -b build_agent --with-build
rem   run_workout_calculator.bat --build build_fast --with-build
rem
rem Notes:
rem   - This wrapper forwards --build-dir to run.py.
rem   - --with-build is still required if you want configure/build stages.
rem   - Only -b/--build are accepted for build dir selection in this wrapper.

set "BUILD_DIR_ARG="
set "FORWARD_ARGS="

:parse_args
if "%~1"=="" goto run_cmd

if /I "%~1"=="-b" (
  if "%~2"=="" (
    echo Error: -b requires a build directory name.>&2
    exit /b 2
  )
  set "BUILD_DIR_ARG=--build-dir %~2"
  shift
  shift
  goto parse_args
)

if /I "%~1"=="--build" (
  if "%~2"=="" (
    echo Error: --build requires a build directory name.>&2
    exit /b 2
  )
  set "BUILD_DIR_ARG=--build-dir %~2"
  shift
  shift
  goto parse_args
)

if /I "%~1"=="--build-dir" (
  echo Error: use -b or --build instead of --build-dir in this wrapper.>&2
  exit /b 2
)

set "FORWARD_ARGS=%FORWARD_ARGS% %1"
shift
goto parse_args

:run_cmd
python run.py --suite workout_calculator --no-format-on-success %BUILD_DIR_ARG% %FORWARD_ARGS%

exit /b %ERRORLEVEL%
