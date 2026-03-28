#!/bin/bash

# --- Release Build (Ninja) launcher ---
# Delegate build orchestration to repo-root Python scripts.

set -eu

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
REPO_ROOT="$( cd "${SCRIPT_DIR}/../../../../.." &> /dev/null && pwd )"
RUNNER="${REPO_ROOT}/tools/run.py"

if [[ ! -f "$RUNNER" ]]; then
  echo "Error: scripts runner not found at ${RUNNER}" >&2
  exit 1
fi

if command -v python >/dev/null 2>&1; then
  PYTHON_BIN="python"
elif command -v python3 >/dev/null 2>&1; then
  PYTHON_BIN="python3"
elif command -v py >/dev/null 2>&1; then
  PYTHON_BIN="py"
else
  echo "Error: Python interpreter not found (tried: python, python3, py)." >&2
  exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
  if [[ -d "/ucrt64/bin" ]]; then
    export PATH="/ucrt64/bin:${PATH}"
  fi
  if [[ -d "/mnt/c/msys64/ucrt64/bin" ]]; then
    export PATH="/mnt/c/msys64/ucrt64/bin:${PATH}"
  fi
fi

if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: cmake not found in PATH. Please install cmake or add it to PATH." >&2
  exit 1
fi

echo "--- Starting Release Build (Ninja) ---"
"$PYTHON_BIN" "$RUNNER" configure --app workout_calculator -- -D CMAKE_BUILD_TYPE=Release -D WARNING_LEVEL=STRICT -D FAST_BUILD=OFF
"$PYTHON_BIN" "$RUNNER" build --app workout_calculator
