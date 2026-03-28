#!/bin/bash

# --- Clang-Format launcher ---
# Delegate format workflow to repo-root Python scripts.

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

echo "--- Running Clang-Format ---"
"$PYTHON_BIN" "$RUNNER" format --app workout_calculator
